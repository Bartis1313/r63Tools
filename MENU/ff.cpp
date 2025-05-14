#include "ff.h"

#include <vector>
#include <d3d11.h>
#include <chrono>
#include "../SDK/sdk.h"
#include "../SDK/ff.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include "widgets.h"

struct ScreenshotData 
{
    std::vector<char> buffer;
    int width = 0;
    int height = 0;
    ID3D11ShaderResourceView* textureSRV = nullptr;
    bool needsUpdate = false;
    std::string playerName;
    std::string timestamp;
};

class ScreenshotManager 
{
private:
    ScreenshotData m_currentScreenshot{ };
    bool m_pendingRequest = false;
    fb::ServerPlayer* m_targetPlayer = nullptr;
    FF::GameBlocksScreenV1 m_format{ };

public:
    ~ScreenshotManager() 
    {
        if (m_currentScreenshot.textureSRV) 
        {
            m_currentScreenshot.textureSRV->Release();
            m_currentScreenshot.textureSRV = nullptr;
        }
    }

    void onScreenshotReceived(fb::ServerPlayer* player, char* buf, unsigned int size) 
    {
        if (!buf)
            return;

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::system_clock::duration tp = now.time_since_epoch();
        tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

        time_t tt = std::chrono::system_clock::to_time_t(now);
        struct tm t;
        localtime_s(&t, &tt);

        char namebuf[MAX_PATH];
        sprintf_s(namebuf, "%04u_%02u_%02u_%02u_%02u_%02u.%03u",
            t.tm_year + 1900,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec,
            static_cast<unsigned>(tp / std::chrono::milliseconds(1)));

        m_currentScreenshot.buffer.assign(buf, buf + size);
        m_currentScreenshot.needsUpdate = true;
        m_currentScreenshot.playerName = player->m_Name.c_str();
        m_currentScreenshot.timestamp = namebuf;
    }

    bool loadTextureFromJPEG(ID3D11Device* device) 
    {
        if (m_currentScreenshot.buffer.empty() || !device) 
            return false;

        if (m_currentScreenshot.textureSRV) 
        {
            m_currentScreenshot.textureSRV->Release();
            m_currentScreenshot.textureSRV = nullptr;
        }

        int width, height, channels;
        stbi_uc* decodedData = stbi_load_from_memory(
            (stbi_uc*)m_currentScreenshot.buffer.data(),
            m_currentScreenshot.buffer.size(),
            &width, &height, &channels, 4);

        if (!decodedData) return false;

        D3D11_TEXTURE2D_DESC desc{ };
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subResource{ };
        subResource.pSysMem = decodedData;
        subResource.SysMemPitch = width * 4;
        subResource.SysMemSlicePitch = 0;

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &subResource, &texture);

        stbi_image_free(decodedData);

        if (FAILED(hr) || !texture)
            return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{ };
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(texture, &srvDesc, &m_currentScreenshot.textureSRV);

        texture->Release();

        if (FAILED(hr)) return false;

        m_currentScreenshot.width = width;
        m_currentScreenshot.height = height;
        m_currentScreenshot.needsUpdate = false;

        return true;
    }

    void renderImGui(ID3D11Device* device) 
    {
        if (m_currentScreenshot.needsUpdate) 
        {
            loadTextureFromJPEG(device);
        }

        // this is hardcoded, client wraps in GetClientRect
        const uint32_t maxScreenW = 2560;
        const uint32_t maxScreenH = 1440;
        const uint32_t minScreen = 0;

        ImGui::SliderScalar("ff width", ImGuiDataType_U32, &m_format.m_width, &minScreen, &maxScreenW);
        ImGui::SliderScalar("ff height", ImGuiDataType_U32, &m_format.m_height, &minScreen, &maxScreenH);
        ImGui::Checkbox("ff adjust aspect ratio", &m_format.m_adjustSizeWithAspectRatio);
        int currentIndex = static_cast<int>(m_format.m_screenshotFormat);
        constexpr auto names = magic_enum::enum_names<FF::ScreenshotFormat>();
        if (ImGui::Combo("Screenshot Format", &currentIndex, [](void* data, int idx, const char** out_text)
            {
                *out_text = names[idx].data();
                return true;
            }, (void*)&names, static_cast<int>(names.size())))
        {
            constexpr auto values = magic_enum::enum_values<FF::ScreenshotFormat>();
            m_format.m_screenshotFormat = values[currentIndex];
        }

        static int idxPlayer = 0;
        const auto players = fb::ServerGameContext::GetInstance()->m_serverPlayerManager->getPlayers();

        if (players->size())
        {
            std::vector<std::string> allNames;
            allNames.reserve(players->size());
            for (size_t i = 0; i < players->size(); ++i)
            {
                if (IsValidPtr(players->at(i)))
                {
                    allNames.emplace_back(players->at(i)->m_Name.c_str());
                }

                if (idxPlayer > i)
                {
                    idxPlayer = 0;
                }
            }

            ImGui::ComboWithFilter("Player names", &idxPlayer, allNames);

            char buf[128];
            sprintf_s(buf, sizeof(buf), "Request screen %s (%i)###RequestScreen", allNames.at(idxPlayer).c_str(), idxPlayer);

            if (ImGui::Button(buf))
            {
                FF::requestScreen(players->at(idxPlayer), m_format, 0);
            }
        }

        if (m_currentScreenshot.textureSRV)
        {
            ImGui::Text("Player: %s", m_currentScreenshot.playerName.c_str());
            ImGui::Text("Timestamp: %s", m_currentScreenshot.timestamp.c_str());

            static float screenshotScale = 0.5f;
            ImGui::SliderFloat("Image Scale", &screenshotScale, 0.1f, 2.0f, "%.1fx", ImGuiSliderFlags_AlwaysClamp);

            ImTextureID screenTexId = reinterpret_cast<ImTextureID>(m_currentScreenshot.textureSRV);
            ImVec2 imageSize = ImVec2(m_format.m_width * screenshotScale, m_format.m_height * screenshotScale);
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);

            ImGui::ImageWithBg(screenTexId, imageSize, uv_min, uv_max, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Save Copy"))
            {
                const auto wantedPath = std::filesystem::current_path() / "FF_SS" / m_currentScreenshot.playerName;
                if (!std::filesystem::exists(wantedPath))
                    std::filesystem::create_directories(wantedPath);

                std::string file = m_currentScreenshot.playerName + std::string{ "_FF_SCREENSHOT_" } +
                    m_currentScreenshot.timestamp + ".jpg";

                std::ofstream stream{ wantedPath / file, std::ofstream::binary };
                if (stream)
                {
                    stream.write(m_currentScreenshot.buffer.data(), m_currentScreenshot.buffer.size());
                    stream.close();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear"))
            {
                if (m_currentScreenshot.textureSRV)
                {
                    m_currentScreenshot.textureSRV->Release();
                    m_currentScreenshot.textureSRV = nullptr;
                }
                m_currentScreenshot.buffer.clear();
                m_currentScreenshot.playerName.clear();
                m_currentScreenshot.timestamp.clear();
            }
        }
        else
            ImGui::Text("No screenshot available.");
    }
};

ScreenshotManager g_ScreenshotManager;

#include "menu.h"

void ff::draw()
{
    g_ScreenshotManager.renderImGui(menu::g_pd3dDevice);
}

void ff::onScreenshotReceived(fb::ServerPlayer* player, char* buf, unsigned int size)
{
    g_ScreenshotManager.onScreenshotReceived(player, buf, size);
}