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

void entity::draw()
{
    static std::string typen;
    static fb::ClassInfo* cInfo;
    static std::vector<fb::TypeInfo*> addr;
    static ImS64 padding = 0x40;

    static ImS64 minPad = 0;
    static ImS64 maxPad = 0x100; // should do some proper finding

    if (ImGui::InputText("Drop ent name", &typen))
    {

    }

    if (ImGui::SliderScalar("Padding for ent (offsetof to link)", ImGuiDataType_S64, &padding, &minPad, &maxPad, "0x%04X"))
    {

    }

    if (ImGui::Button("Dump ptr"))
    {
        addr.clear();
        fb::ClassInfo* cInfo = findClassInfo(typen.c_str());

        for (auto el : fb::EntityList<fb::TypeInfo>{ cInfo })
        {
            addr.push_back(el);
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

    ImGui::Text("Found %i ents", addr.size());

    // do some memory view here...
}