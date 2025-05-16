#include "entity.h"

#include "../SDK/sdk.h"
#include "widgets.h"
#include <imgui_stdlib.h>

static fb::ClassInfo* findClassInfo(const char* className)
{
    const auto pFirstTypeInfo = OFFSET_FIRSTTYPEINFO;
    auto* ppTypesList = reinterpret_cast<fb::TypeInfo**>(pFirstTypeInfo);

    if (!ppTypesList || !IsValidPtr(*ppTypesList))
        return nullptr;

    fb::TypeInfo* ti = *ppTypesList;

    for (; ti != nullptr; ti = ti->m_Next) 
    {
        if (!IsValidPtr(ti->m_InfoData) || !IsValidPtr(ti->m_InfoData->m_Name))
            continue;

        if (_stricmp(ti->m_InfoData->m_Name, className) != 0)
            continue;

        if (ti->GetTypeCode() != fb::BasicTypesEnum::kTypeCode_Class)
            continue;

        return static_cast<fb::ClassInfo*>(ti);
    }

    return nullptr;
}

// this SHOULD be found manually, or as ida plugin.
// currently just as example
// or dump link once, and find "same" looking heap?
uintptr_t findIterableOffset(uintptr_t entityAddress, uintptr_t validEntityPoolStart, uintptr_t validEntityPoolEnd) 
{
    constexpr uintptr_t maxOffset = 0x100; // have to hardcode it

    for (uintptr_t offset = /*sizeof(fb::Entity)*/0x20; offset < maxOffset; offset += sizeof(PVOID)) // or padding better be 4
    {
        uintptr_t maybePrev = *(uintptr_t*)(entityAddress + offset);
        uintptr_t maybeNext = *(uintptr_t*)(entityAddress + offset + sizeof(uintptr_t));

        if ((maybePrev & 0xF) != 0 || (maybeNext & 0xF) != 0)
            continue;

        if (maybePrev >= validEntityPoolStart && maybePrev < validEntityPoolEnd &&
            maybeNext >= validEntityPoolStart && maybeNext < validEntityPoolEnd) 
        {

            uintptr_t prevsNext = *(uintptr_t*)(maybePrev + offset + sizeof(uintptr_t));
            uintptr_t nextsPrev = *(uintptr_t*)(maybeNext + offset);

            if (prevsNext == entityAddress && nextsPrev == entityAddress) 
            {
                return offset;
            }
        }
    }

    return 0;
}

void drawMemoryEditor(void* baseAddress, int size, int dataTypeSize, bool showTypes, bool useHexOffset);

static int bytesPerRow = 16;
static int memoryViewSize = 0x200;
static int selectedEntity = -1;
static bool showTypes = true;
static int dataTypeSize = 4; // Default to 4 bytes (int, float)
static bool useHexOffset = true;

void entity::draw()
{
    static std::string typen;
    static fb::ClassInfo* cInfo;
    static std::vector<fb::TypeInfo*> addr;
    static ImS64 padding = 0x40;

    static ImS64 minPad = 0;
    static ImS64 maxPad = 0x100; // should do some proper finding

    static bool includeOnlyIterable = true;

#ifdef _WIN64
    if (ImGui::SliderScalar("Padding for ent (offsetof to link)", ImGuiDataType_S64, &padding, &minPad, &maxPad, "0x%04X"))
    {

    }
#elif _WIN32
    ImGui::Checkbox("includeOnlyIterable", &includeOnlyIterable);
#endif

    if (ImGui::InputTextWithHint("Drop ent name", "Press enter to dump", &typen, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        addr.clear();
        selectedEntity = -1;
        cInfo = findClassInfo(typen.c_str());
        if (cInfo)
        {

#ifdef _WIN64
            for (auto el : fb::EntityList<fb::TypeInfo>{ cInfo, padding })
            {
                addr.push_back(el);
            }
#elif _WIN32
            for (auto el : fb::EntityList<fb::TypeInfo>{ cInfo, includeOnlyIterable })
            {
                addr.push_back(el);
            }
#endif
    }
    }

    if (!cInfo)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "classinfo for %s not found", typen.c_str());
        return;
    }

    ImGui::TextColored(ImVec4(0, 1, 0, 1), "classinfo at %p", cInfo);

    if (addr.empty())
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "no entities found...", typen.c_str());
        return;
    }

    ImGui::Text("Found %zu entities", addr.size());
    
    std::vector<std::string> instanceLabels;
    instanceLabels.reserve(addr.size());

    for (size_t i = 0; i < addr.size(); i++)
    {
        char buffer[32]; 
        snprintf(buffer, sizeof(buffer), "[%zu] 0x%p", i, addr[i]);
        instanceLabels.push_back(buffer);
    }

    ImGui::ComboWithFilter("##ResourceInstance", &selectedEntity, instanceLabels);

    ImGui::Separator();
    if (ImGui::TreeNode("Memory Viewer Options"))
    {
        ImGui::SliderInt("Memory View Size", &memoryViewSize, 0x100, 0x1000);
        ImGui::Checkbox("Show Type Preview", &showTypes);

        ImGui::Text("Data Column Size");
        ImGui::RadioButton("1 byte", &dataTypeSize, 1); ImGui::SameLine();
        ImGui::RadioButton("2 bytes", &dataTypeSize, 2); ImGui::SameLine();
        ImGui::RadioButton("4 bytes", &dataTypeSize, 4); ImGui::SameLine();
        ImGui::RadioButton("8 bytes", &dataTypeSize, 8);

        ImGui::Checkbox("Use Hex Offset Display", &useHexOffset);
        ImGui::TreePop();
    }

    if (selectedEntity >= 0 && selectedEntity < addr.size())
    {
        void* baseAddress = addr[selectedEntity];
        ImGui::Separator();
        ImGui::Text("Memory View for Entity %d at %p", selectedEntity, baseAddress);

        drawMemoryEditor(baseAddress, memoryViewSize, dataTypeSize, showTypes, useHexOffset);
    }
}

// is it enough?
bool isHeapPointer(const void* ptr)
{
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (address > 0x10000) && (address < 0x7FFFFFFFFFFF) && ((address & 0xF) == 0);
}

void drawMemoryEditor(void* baseAddress, int size, int dataTypeSize, bool showTypes, bool useHexOffset)
{
    int columnsPerRow = 16 / dataTypeSize;

    ImGui::BeginChild("##memory_viewer", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::BeginTable("##memory_table", showTypes ? 3 : 2,
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthStretch);
        if (showTypes)
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 160.0f);

        ImGui::TableHeadersRow();

        for (int offset = 0; offset < size; offset += 16)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(useHexOffset ? "0x%04X" : "%04d", offset);

            ImGui::TableSetColumnIndex(1);

            for (int col = 0; col < columnsPerRow; col++)
            {
                int colOffset = offset + col * dataTypeSize;

                if (colOffset >= size)
                    break;

                ImVec2 cellPos = ImGui::GetCursorScreenPos();
                switch (dataTypeSize)
                {
                case 1: // Byte
                    if (colOffset < size)
                    {
                        uint8_t value = *((uint8_t*)baseAddress + colOffset);
                        ImGui::Text("%02X", value);
                    }
                    break;

                case 2: // Word (2 bytes)
                    if (colOffset + 1 < size)
                    {
                        uint16_t value = *((uint16_t*)((uint8_t*)baseAddress + colOffset));
                        ImGui::Text("%04X", value);
                    }
                    break;

                case 4: // Dword (4 bytes)
                    if (colOffset + 3 < size)
                    {
                        uint32_t value = *((uint32_t*)((uint8_t*)baseAddress + colOffset));
                        ImGui::Text("%08X", value);
                    }
                    break;

                case 8: // Qword (8 bytes)
                    if (colOffset + 7 < size)
                    {
                        uint64_t value = *((uint64_t*)((uint8_t*)baseAddress + colOffset));
                        bool isPointer = isHeapPointer((void*)value);

                        if (isPointer)
                            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%016llX", value);
                        else
                            ImGui::Text("%016llX", value);
                    }
                    break;
                }

                ImVec2 cellEndPos = ImVec2(
                    cellPos.x + ImGui::GetItemRectSize().x,
                    cellPos.y + ImGui::GetItemRectSize().y
                );

                if (ImGui::IsMouseHoveringRect(cellPos, cellEndPos))
                {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    drawList->AddRect(cellPos, cellEndPos,
                        IM_COL32(255, 255, 0, 255),
                        0.0f, ImDrawFlags_None, 2.0f);

                    if (colOffset + 7 < size) // enough bytes?
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Offset: 0x%X", colOffset);
                        ImGui::Separator();

                        // small preview
                        int8_t int8Value = *((int8_t*)((uint8_t*)baseAddress + colOffset));
                        uint8_t uint8Value = *((uint8_t*)((uint8_t*)baseAddress + colOffset));
                        char charValue = *((char*)((uint8_t*)baseAddress + colOffset));

                        ImGui::Text("int8:   %d", int8Value);
                        ImGui::Text("uint8:  %u (0x%02X)", uint8Value, uint8Value);
                        ImGui::Text("char:   %c", (charValue >= 32 && charValue < 127) ? charValue : '.');

                        if (colOffset + 1 < size)
                        {
                            int16_t int16Value = *((int16_t*)((uint8_t*)baseAddress + colOffset));
                            uint16_t uint16Value = *((uint16_t*)((uint8_t*)baseAddress + colOffset));

                            ImGui::Text("int16:  %d", int16Value);
                            ImGui::Text("uint16: %u (0x%04X)", uint16Value, uint16Value);
                        }

                        if (colOffset + 3 < size)
                        {
                            int32_t int32Value = *((int32_t*)((uint8_t*)baseAddress + colOffset));
                            uint32_t uint32Value = *((uint32_t*)((uint8_t*)baseAddress + colOffset));
                            float floatValue = *((float*)((uint8_t*)baseAddress + colOffset));

                            ImGui::Text("int32:  %d", int32Value);
                            ImGui::Text("uint32: %u (0x%08X)", uint32Value, uint32Value);
                            ImGui::Text("float:  %.6f", floatValue);
                        }

                        if (colOffset + 7 < size)
                        {
                            int64_t int64Value = *((int64_t*)((uint8_t*)baseAddress + colOffset));
                            uint64_t uint64Value = *((uint64_t*)((uint8_t*)baseAddress + colOffset));
                            double doubleValue = *((double*)((uint8_t*)baseAddress + colOffset));

                            ImGui::Text("int64:  %lld", int64Value);
                            ImGui::Text("uint64: %llu (0x%016llX)", uint64Value, uint64Value);
                            ImGui::Text("double: %.6f", doubleValue);

                            if (isHeapPointer((void*)uint64Value))
                            {
                                ImGui::Separator();
                                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Potential heap pointer: %p", (void*)uint64Value);

                                MEMORY_BASIC_INFORMATION mbi;
                                if (VirtualQuery((void*)uint64Value, &mbi, sizeof(mbi)) &&
                                    (mbi.State & MEM_COMMIT) &&
                                    (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
                                {
                                    ImGui::Text("Content: %016llX %016llX",
                                        *((uint64_t*)uint64Value),
                                        *((uint64_t*)uint64Value + 1));
                                }
                                else
                                {
                                    ImGui::Text("Cannot read pointer target (invalid memory)");
                                }
                            }
                        }

                        ImGui::EndTooltip();
                    }
                }

                if (col < columnsPerRow - 1 && colOffset + dataTypeSize < size)
                    ImGui::SameLine();
            }

            // ascii
            if (showTypes)
            {
                ImGui::TableSetColumnIndex(2);

                char asciiBuffer[17] = { 0 };
                for (int i = 0; i < 16 && offset + i < size; i++)
                {
                    uint8_t value = *((uint8_t*)baseAddress + offset + i);
                    asciiBuffer[i] = (value >= 32 && value < 127) ? value : '.';
                }

                ImGui::Text("%s", asciiBuffer);
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}