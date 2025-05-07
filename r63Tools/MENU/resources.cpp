#include "resources.h"

#include "../SDK/sdk.h"
#include <imgui.h>
#include "widgets.h"

#include <algorithm>

void MinimalResourceEditor::Initialize(FBBackend::SDK_GENERATOR* sdkGenerator)
{
    m_sdkGenerator = sdkGenerator;
    UpdateResources();

    m_valid = true;
}

void MinimalResourceEditor::Destroy()
{
    m_valid = false;
}

void MinimalResourceEditor::UpdateResources()
{
    m_resources.clear();
    m_resourceTypes.clear();

    fb::ResourceManager* prm = fb::ResourceManager::GetInstance();
    if (!prm)
        return;

    for (size_t i = 0; i < prm->compartmenstSize(); ++i)
    {
        const auto compartment = prm->m_compartments[i];
        if (!IsValidPtr(compartment))
            continue;

        for (const auto obj : compartment->m_objects)
        {
            if (!IsValidPtr(obj))
                continue;

            const std::string name = obj->GetType()->GetTypeInfoData()->m_Name;
            if (name.empty())
                continue;

            m_resources[name].push_back(obj);
        }
    }

    for (const auto& [type, instances] : m_resources)
    {
        if (!instances.empty())
            m_resourceTypes.push_back(type);
    }

    std::sort(m_resourceTypes.begin(), m_resourceTypes.end());
}

std::string MinimalResourceEditor::GetActualClassName(void* instance, const std::string& defaultName)
{
    if (!instance)
        return defaultName;

    fb::ITypedObject* typedObj = static_cast<fb::ITypedObject*>(instance);
    if (IsValidPtr(typedObj) && IsValidPtr(typedObj->GetType()))
    {
        fb::TypeInfo* type = typedObj->GetType();
        if (IsValidPtr(type->GetTypeInfoData()))
        {
            const char* name = type->GetTypeInfoData()->m_Name;
            if (name && name[0] != '\0')
            {
                return std::string(name);
            }
        }
    }

    return defaultName;
}

void MinimalResourceEditor::Render()
{
    if (ImGui::Button("Refresh Resources"))
    {
        UpdateResources();
        m_selectedInstanceIndex = -1;
    }

    ImGui::SameLine();
    ImGui::Text("Found %zu resource types", m_resourceTypes.size());

    ImGui::Text("Select Resource Type:");
    bool typeChanged = ImGui::ComboWithFilter("##ResourceType", &m_selectedTypeIndex, m_resourceTypes);
    if (typeChanged)
    {
        m_selectedInstanceIndex = -1;
    }

    if (m_selectedTypeIndex >= 0 && m_selectedTypeIndex < m_resourceTypes.size())
    {
        const std::string& typeName = m_resourceTypes[m_selectedTypeIndex];
        const auto& instances = m_resources[typeName];

        ImGui::Separator();
        ImGui::Text("Select %s Instance:", typeName.c_str());
        ImGui::Text("Found %zu instances", instances.size());

        std::vector<std::string> instanceLabels(instances.size());
        for (size_t i = 0; i < instances.size(); i++)
        {
            struct PseudoAsset
            {
                char pad[16];
                char* m_Name;
            };

            char buffer[256];
            PseudoAsset* asset = reinterpret_cast<PseudoAsset*>(instances[i]);
            snprintf(buffer, sizeof(buffer), "[%zu] 0x%p (%s)", i, instances[i], asset->m_Name);
            instanceLabels.push_back(buffer);
        }

        ImGui::ComboWithFilter("##ResourceInstance", &m_selectedInstanceIndex, instanceLabels);

        if (m_selectedInstanceIndex >= 0 && m_selectedInstanceIndex < instances.size())
        {
            void* instance = instances[m_selectedInstanceIndex];

            ImGui::Separator();
            ImGui::Text("Editing: %s @ %s", typeName.c_str(), instanceLabels[m_selectedInstanceIndex].c_str());

            ImGui::BeginChild("Editor", ImVec2(0, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
            bool modified = RenderObjectEditor(typeName, instance);
            ImGui::EndChild();

            if (modified)
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Resource modified!");
            }
        }
    }
}

bool MinimalResourceEditor::RenderObjectEditor(const std::string& typeName, void* instance)
{
    if (!instance || !m_sdkGenerator)
        return false;

    bool modified = false;

    FBBackend::SDK_CLASS* sdkClass = FindClass(typeName);
    if (sdkClass)
    {
        if (sdkClass->m_parent)
        {
            modified |= RenderObjectEditor(sdkClass->m_parent->m_name, instance);
        }

        if (ImGui::CollapsingHeader(typeName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();

            for (const auto& member : sdkClass->m_sdkClassMembers)
            {
                if (!member.m_isPad)
                {
                    modified |= RenderField(instance, member.m_name, member.m_type,
                        member.m_offset, member.m_size);
                }
            }

            ImGui::Unindent();
        }

        return modified;
    }

    FBBackend::SDK_STRUCT* sdkStruct = FindStruct(typeName);
    if (sdkStruct)
    {
        if (ImGui::CollapsingHeader(typeName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();

            for (const auto& member : sdkStruct->m_sdkStructMembers)
            {
                if (!member.m_isPad)
                {
                    modified |= RenderField(instance, member.m_name, member.m_type,
                        member.m_offset, member.m_size);
                }
            }

            ImGui::Unindent();
        }

        return modified;
    }

    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Unknown type: %s", typeName.c_str());
    return false;
}

FBBackend::SDK_CLASS* MinimalResourceEditor::FindClass(const std::string& name)
{
    if (!m_sdkGenerator)
        return nullptr;

    for (auto* cls : *m_sdkGenerator->m_sdkClasses)
    {
        if (cls->m_name == name)
            return cls;
    }

    return nullptr;
}

FBBackend::SDK_STRUCT* MinimalResourceEditor::FindStruct(const std::string& name)
{
    if (!m_sdkGenerator)
        return nullptr;

    for (auto* str : *m_sdkGenerator->m_sdkStructs)
    {
        if (str->m_name == name)
            return str;
    }

    return nullptr;
}

FBBackend::SDK_ENUM* MinimalResourceEditor::FindEnum(const std::string& name)
{
    if (!m_sdkGenerator)
        return nullptr;

    for (auto* enm : *m_sdkGenerator->m_sdkEnums)
    {
        if (enm->m_name == name)
            return enm;
    }

    return nullptr;
}

bool MinimalResourceEditor::RenderField(void* basePtr, const std::string& fieldName, const std::string& fieldType,
    unsigned int offset, unsigned int size)
{
    ImGui::PushID((void*)(uintptr_t)(offset));

    bool modified = false;

    if (fieldType == "bool" ||
        fieldType == "__int8" || fieldType == "unsigned __int8" ||
        fieldType == "__int16" || fieldType == "unsigned __int16" ||
        fieldType == "__int32" || fieldType == "unsigned __int32" ||
        fieldType == "__int64" || fieldType == "unsigned __int64" ||
        fieldType == "float" || fieldType == "double" || fieldType == "char*")
    {
        modified = RenderBasicType(basePtr, fieldName, fieldType, offset);
    }
    else if (FindEnum(fieldType))
    {
        modified = RenderEnum(basePtr, fieldName, fieldType, offset);
    }
    else if (fieldType.back() == '*')
    {
        std::string className = fieldType.substr(0, fieldType.length() - 1);
        modified = RenderClassPointer(basePtr, fieldName, className, offset);
    }
    else if (FindStruct(fieldType))
    {
        modified = RenderStruct(basePtr, fieldName, fieldType, offset);
    }
    else if (fieldType.find("Array<") == 0)
    {
        modified = RenderArray(basePtr, fieldName, fieldType, offset, size);
    }
    else
    {
        ImGui::Text("%s: %s [Unknown type @ 0x%X]", fieldName.c_str(), fieldType.c_str(), offset);
    }

    ImGui::PopID();
    return modified;
}

bool MinimalResourceEditor::RenderBasicType(void* basePtr, const std::string& fieldName, const std::string& fieldType,
    unsigned int offset)
{
    void* fieldPtr = static_cast<char*>(basePtr) + offset;
    bool modified = false;

    char label[256];
    snprintf(label, sizeof(label), "%s (%s)", fieldName.c_str(), fieldType.c_str());

    if (fieldType == "bool")
    {
        bool* value = static_cast<bool*>(fieldPtr);
        modified = ImGui::Checkbox(label, value);
    }
    else if (fieldType == "__int8")
    {
        int8_t* value = static_cast<int8_t*>(fieldPtr);
        int temp = static_cast<int>(*value);
        if (ImGui::InputInt(label, &temp))
        {
            *value = static_cast<int8_t>(std::clamp(temp, INT8_MIN, (int)INT8_MAX));
            modified = true;
        }
    }
    else if (fieldType == "unsigned __int8")
    {
        uint8_t* value = static_cast<uint8_t*>(fieldPtr);
        int temp = static_cast<int>(*value);
        if (ImGui::InputInt(label, &temp))
        {
            *value = static_cast<uint8_t>(std::clamp(temp, 0, (int)UINT8_MAX));
            modified = true;
        }
    }
    else if (fieldType == "__int16")
    {
        int16_t* value = static_cast<int16_t*>(fieldPtr);
        int temp = static_cast<int>(*value);
        if (ImGui::InputInt(label, &temp))
        {
            *value = static_cast<int16_t>(std::clamp(temp, INT16_MIN, (int)INT16_MAX));
            modified = true;
        }
    }
    else if (fieldType == "unsigned __int16")
    {
        uint16_t* value = static_cast<uint16_t*>(fieldPtr);
        int temp = static_cast<int>(*value);
        if (ImGui::InputInt(label, &temp))
        {
            *value = static_cast<uint16_t>(std::clamp(temp, 0, (int)UINT16_MAX));
            modified = true;
        }
    }
    else if (fieldType == "__int32")
    {
        int32_t* value = static_cast<int32_t*>(fieldPtr);
        modified = ImGui::InputInt(label, value);
    }
    else if (fieldType == "unsigned __int32")
    {
        uint32_t* value = static_cast<uint32_t*>(fieldPtr);
        int temp = static_cast<int>(*value);
        if (ImGui::InputInt(label, &temp))
        {
            *value = static_cast<uint32_t>(max(0, temp));
            modified = true;
        }
    }
    else if (fieldType == "__int64")
    {
        int64_t* value = static_cast<int64_t*>(fieldPtr);
        modified = ImGui::InputScalar(label, ImGuiDataType_S64, value);
    }
    else if (fieldType == "unsigned __int64")
    {
        uint64_t* value = static_cast<uint64_t*>(fieldPtr);
        modified = ImGui::InputScalar(label, ImGuiDataType_U64, value);
    }
    else if (fieldType == "float")
    {
        float* value = static_cast<float*>(fieldPtr);
        modified = ImGui::InputFloat(label, value);
    }
    else if (fieldType == "double")
    {
        double* value = static_cast<double*>(fieldPtr);
        modified = ImGui::InputDouble(label, value);
    }
    else if (fieldType == "char*")
    {
        char** strPtr = static_cast<char**>(fieldPtr);
        ImGui::Text("%s: %s", label, *strPtr ? *strPtr : "null");
    }
    else
    {
        ImGui::Text("%s: [Unsupported basic type]", label);
    }

    return modified;
}

bool MinimalResourceEditor::RenderEnum(void* basePtr, const std::string& fieldName, const std::string& enumType,
    unsigned int offset)
{
    FBBackend::SDK_ENUM* sdkEnum = FindEnum(enumType);
    if (!sdkEnum)
    {
        ImGui::Text("%s: [Enum %s not found]", fieldName.c_str(), enumType.c_str());
        return false;
    }

    // Enums are typically stored as int32
    int32_t* enumValuePtr = reinterpret_cast<int32_t*>(static_cast<char*>(basePtr) + offset);
    int currentValue = *enumValuePtr;
    bool modified = false;

    // Find current value name
    std::string currentValueName = "Unknown";
    for (const auto& field : sdkEnum->m_sdkEnumFields)
    {
        if (field.m_offset == currentValue)
        {
            currentValueName = field.m_name;
            break;
        }
    }

    char label[256];
    snprintf(label, sizeof(label), "%s (%s)", fieldName.c_str(), enumType.c_str());

    // Create combo box for enum
    if (ImGui::BeginCombo(label, currentValueName.c_str()))
    {
        for (const auto& field : sdkEnum->m_sdkEnumFields)
        {
            bool isSelected = (field.m_offset == currentValue);
            if (ImGui::Selectable(field.m_name.c_str(), isSelected))
            {
                *enumValuePtr = field.m_offset;
                modified = true;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    return modified;
}

bool MinimalResourceEditor::RenderClassPointer(void* basePtr, const std::string& fieldName, const std::string& className,
    unsigned int offset)
{
    void** ptrLocation = reinterpret_cast<void**>(static_cast<char*>(basePtr) + offset);
    void* ptr = *ptrLocation;

    char label[256];
    snprintf(label, sizeof(label), "%s (%s*)", fieldName.c_str(), className.c_str());

    bool modified = false;

    if (ptr && IsValidPtr(ptr))
    {
        ImGui::Text("%s: 0x%p", label, ptr);

        ImGui::SameLine();
        if (ImGui::Button("Explore##btn"))
        {
            ImGui::OpenPopup("PointerExplorer");
        }

        ImGuiContext& g = *GImGui;
        // forced, because flags are overwritten
        if (ImGui::BeginPopupEx(g.CurrentWindow->GetID("PointerExplorer"), 0))
        {
            ImGui::Text("Exploring: %s", label);
            ImGui::Separator();

            std::string actualClassName = GetActualClassName(ptr, className);
            ImGui::Text("Actual type: %s", actualClassName.c_str());

            ImGui::BeginChild("PointerContent", ImVec2(0, 300), ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);

            FBBackend::SDK_CLASS* sdkClass = FindClass(actualClassName);
            if (sdkClass)
            {
                modified |= RenderObjectEditor(actualClassName, ptr);
            }
            else
            {
                ImGui::Text("Class %s not found in SDK", className.c_str());
            }

            ImGui::EndChild();

            ImGui::EndPopup();
        }
    }
    else
    {
        ImGui::Text("%s: nullptr", label);
    }

    return modified;
}
bool MinimalResourceEditor::RenderStruct(void* basePtr, const std::string& fieldName, const std::string& structName,
    unsigned int offset)
{
    void* structPtr = static_cast<char*>(basePtr) + offset;

    char label[256];
    snprintf(label, sizeof(label), "%s (%s)", fieldName.c_str(), structName.c_str());

    bool modified = false;
    bool headerOpen = ImGui::TreeNode(label);

    if (headerOpen)
    {
        FBBackend::SDK_STRUCT* sdkStruct = FindStruct(structName);
        if (sdkStruct)
        {
            for (const auto& member : sdkStruct->m_sdkStructMembers)
            {
                if (!member.m_isPad)
                {
                    modified |= RenderField(structPtr, member.m_name, member.m_type, member.m_offset, member.m_size);
                }
            }
        }
        else
        {
            ImGui::Text("Struct %s not found in SDK", structName.c_str());
        }

        ImGui::TreePop();
    }

    return modified;
}

bool MinimalResourceEditor::RenderArray(void* basePtr, const std::string& fieldName, const std::string& arrayType,
    unsigned int offset, unsigned int size)
{
    void* arrayPtr = static_cast<char*>(basePtr) + offset;

    // "Array<float>" -> "float"
    std::string elementType;
    size_t start = arrayType.find('<') + 1;
    size_t end = arrayType.find('>');
    if (start != std::string::npos && end != std::string::npos && start < end)
    {
        elementType = arrayType.substr(start, end - start);
    }

    char label[256];
    snprintf(label, sizeof(label), "%s (%s)", fieldName.c_str(), arrayType.c_str());

    bool modified = false;
    bool headerOpen = ImGui::TreeNode(label);

    if (headerOpen)
    {
        fb::ArrayBase* array = static_cast<fb::ArrayBase*>(arrayPtr);
        void* firstElement = array->m_firstElement;

        fb::ArrayHeader* header = IsValidPtr(firstElement) ? array->GetHeader() : nullptr;
        int32_t arraySize = header ? header->size : 0;

        ImGui::Text("Size: %d", arraySize);
        ImGui::Text("First Element: 0x%p", firstElement);

        if (firstElement && IsValidPtr(firstElement) && arraySize > 0)
        {
            size_t elementSize = 0;

            if (elementType == "bool") elementSize = sizeof(bool);
            else if (elementType == "__int8") elementSize = sizeof(int8_t);
            else if (elementType == "unsigned __int8") elementSize = sizeof(uint8_t);
            else if (elementType == "__int16") elementSize = sizeof(int16_t);
            else if (elementType == "unsigned __int16") elementSize = sizeof(uint16_t);
            else if (elementType == "__int32") elementSize = sizeof(int32_t);
            else if (elementType == "unsigned __int32") elementSize = sizeof(uint32_t);
            else if (elementType == "__int64") elementSize = sizeof(int64_t);
            else if (elementType == "unsigned __int64") elementSize = sizeof(uint64_t);
            else if (elementType == "float") elementSize = sizeof(float);
            else if (elementType == "double") elementSize = sizeof(double);
            else if (elementType.back() == '*') elementSize = sizeof(void*);
            else
            {
                FBBackend::SDK_STRUCT* sdkStruct = FindStruct(elementType);
                if (sdkStruct)
                {
                    elementSize = sdkStruct->m_size;
                }
                else
                {
                    FBBackend::SDK_CLASS* sdkClass = FindClass(elementType);
                    if (sdkClass)
                    {
                        elementSize = sdkClass->m_size;
                    }
                    else
                    {
                        elementSize = sizeof(void*);
                    }
                }
            }

            ImGui::BeginChild("ArrayElements", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
            {
                for (int i = 0; i < arraySize; ++i)
                {
                    char elementLabel[32];
                    snprintf(elementLabel, sizeof(elementLabel), "[%d]", i);

                    ImGui::PushID(i);

                    void* elementPtr = (void*)((intptr_t)firstElement + (i * elementSize));

                    if (elementType == "bool" ||
                        elementType == "__int8" || elementType == "unsigned __int8" ||
                        elementType == "__int16" || elementType == "unsigned __int16" ||
                        elementType == "__int32" || elementType == "unsigned __int32" ||
                        elementType == "__int64" || elementType == "unsigned __int64" ||
                        elementType == "float" || elementType == "double" || elementType == "char*")
                    {
                        if (elementType == "bool")
                        {
                            bool* value = static_cast<bool*>(elementPtr);
                            modified |= ImGui::Checkbox(elementLabel, value);
                        }
                        else if (elementType == "__int8")
                        {
                            int8_t* value = static_cast<int8_t*>(elementPtr);
                            int temp = static_cast<int>(*value);
                            if (ImGui::InputInt(elementLabel, &temp))
                            {
                                *value = static_cast<int8_t>(std::clamp(temp, INT8_MIN, (int)INT8_MAX));
                                modified = true;
                            }
                        }
                        else if (elementType == "unsigned __int8")
                        {
                            uint8_t* value = static_cast<uint8_t*>(elementPtr);
                            int temp = static_cast<int>(*value);
                            if (ImGui::InputInt(elementLabel, &temp))
                            {
                                *value = static_cast<uint8_t>(std::clamp(temp, 0, (int)UINT8_MAX));
                                modified = true;
                            }
                        }
                        else if (elementType == "__int16")
                        {
                            int16_t* value = static_cast<int16_t*>(elementPtr);
                            int temp = static_cast<int>(*value);
                            if (ImGui::InputInt(elementLabel, &temp))
                            {
                                *value = static_cast<int16_t>(std::clamp(temp, INT16_MIN, (int)INT16_MAX));
                                modified = true;
                            }
                        }
                        else if (elementType == "unsigned __int16")
                        {
                            uint16_t* value = static_cast<uint16_t*>(elementPtr);
                            int temp = static_cast<int>(*value);
                            if (ImGui::InputInt(elementLabel, &temp))
                            {
                                *value = static_cast<uint16_t>(std::clamp(temp, 0, (int)UINT16_MAX));
                                modified = true;
                            }
                        }
                        else if (elementType == "__int32")
                        {
                            int32_t* value = static_cast<int32_t*>(elementPtr);
                            modified |= ImGui::InputInt(elementLabel, value);
                        }
                        else if (elementType == "unsigned __int32")
                        {
                            uint32_t* value = static_cast<uint32_t*>(elementPtr);
                            int temp = static_cast<int>(*value);
                            if (ImGui::InputInt(elementLabel, &temp))
                            {
                                *value = static_cast<uint32_t>(max(0, temp));
                                modified = true;
                            }
                        }
                        else if (elementType == "__int64")
                        {
                            int64_t* value = static_cast<int64_t*>(elementPtr);
                            modified |= ImGui::InputScalar(elementLabel, ImGuiDataType_S64, value);
                        }
                        else if (elementType == "unsigned __int64")
                        {
                            uint64_t* value = static_cast<uint64_t*>(elementPtr);
                            modified |= ImGui::InputScalar(elementLabel, ImGuiDataType_U64, value);
                        }
                        else if (elementType == "float")
                        {
                            float* value = static_cast<float*>(elementPtr);
                            modified |= ImGui::InputFloat(elementLabel, value);
                        }
                        else if (elementType == "double")
                        {
                            double* value = static_cast<double*>(elementPtr);
                            modified |= ImGui::InputDouble(elementLabel, value);
                        }
                        else if (elementType == "char*")
                        {
                            char** strPtr = static_cast<char**>(elementPtr);
                            ImGui::Text("%s: %s", elementLabel, *strPtr ? *strPtr : "null");
                        }
                    }
                    else if (FindEnum(elementType))
                    {
                        int32_t* enumValuePtr = static_cast<int32_t*>(elementPtr);
                        FBBackend::SDK_ENUM* sdkEnum = FindEnum(elementType);

                        if (sdkEnum)
                        {
                            int currentValue = *enumValuePtr;

                            std::string currentValueName = "Unknown";
                            for (const auto& field : sdkEnum->m_sdkEnumFields)
                            {
                                if (field.m_offset == currentValue)
                                {
                                    currentValueName = field.m_name;
                                    break;
                                }
                            }

                            if (ImGui::BeginCombo(elementLabel, currentValueName.c_str()))
                            {
                                for (const auto& field : sdkEnum->m_sdkEnumFields)
                                {
                                    bool isSelected = (field.m_offset == currentValue);
                                    if (ImGui::Selectable(field.m_name.c_str(), isSelected))
                                    {
                                        *enumValuePtr = field.m_offset;
                                        modified = true;
                                    }

                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }

                                ImGui::EndCombo();
                            }
                        }
                        else
                        {
                            ImGui::Text("%s: [Enum %s not found]", elementLabel, elementType.c_str());
                        }
                    }
                    else if (elementType.back() == '*')
                    {
                        void** ptrPtr = static_cast<void**>(elementPtr);
                        void* ptr = *ptrPtr;

                        if (ptr && IsValidPtr(ptr))
                        {
                            std::string className = elementType.substr(0, elementType.length() - 1);
                            std::string actualClassName = GetActualClassName(ptr, className);

                            bool isDerived = (actualClassName != className);

                            if (ImGui::TreeNode(elementLabel))
                            {
                                if (isDerived)
                                {
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                        "Actual type: %s", actualClassName.c_str());
                                }

                                FBBackend::SDK_CLASS* sdkClass = FindClass(actualClassName);
                                if (sdkClass)
                                {
                                    modified |= RenderObjectEditor(actualClassName, ptr);
                                }
                                else
                                {
                                    sdkClass = FindClass(className);
                                    if (sdkClass)
                                    {
                                        modified |= RenderObjectEditor(className, ptr);
                                    }
                                    else
                                    {
                                        ImGui::Text("Class %s not found in SDK", className.c_str());
                                    }
                                }

                                ImGui::TreePop();
                            }
                            else
                            {
                                ImGui::SameLine();
                                if (isDerived)
                                {
                                    ImGui::Text("%s (0x%p)", actualClassName.c_str(), ptr);
                                }
                                else
                                {
                                    ImGui::Text("0x%p", ptr);
                                }
                            }
                        }
                        else
                        {
                            ImGui::Text("%s: nullptr", elementLabel);
                        }
                    }
                    else if (FindStruct(elementType))
                    {
                        if (ImGui::TreeNode(elementLabel))
                        {
                            modified |= RenderObjectEditor(elementType, elementPtr);
                            ImGui::TreePop();
                        }
                    }
                    else if (FindClass(elementType))
                    {
                        std::string actualClassName = GetActualClassName(elementPtr, elementType);
                        bool isDerived = (actualClassName != elementType);

                        if (ImGui::TreeNode(elementLabel))
                        {
                            if (isDerived)
                            {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                    "Actual type: %s", actualClassName.c_str());
                            }

                            modified |= RenderObjectEditor(actualClassName, elementPtr);
                            ImGui::TreePop();
                        }
                        else
                        {
                            ImGui::SameLine();
                            if (isDerived)
                            {
                                ImGui::Text("%s", actualClassName.c_str());
                            }
                            else
                            {
                                ImGui::Text("0x%p", elementPtr);
                            }
                        }
                    }
                    else
                    {
                        ImGui::Text("%s: Unknown type %s", elementLabel, elementType.c_str());
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("Array is empty or first element pointer is invalid");
        }

        ImGui::TreePop();
    }

    return modified;
}