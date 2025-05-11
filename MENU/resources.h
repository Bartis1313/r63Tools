#pragma once

#include "../GEN/sdkGen.h"

#include <set>
#include <map>
#include <vector>
#include <string>

namespace fb
{
    class ResourceManager;
}

class MinimalResourceEditor
{
public:
    void Initialize(FBBackend::SDK_GENERATOR* sdkGenerator);
    void Render();
    void Destroy();

private:
    bool m_valid = false;

    FBBackend::SDK_GENERATOR* m_sdkGenerator = nullptr;

    std::map<std::string, std::vector<void*>> m_resources;
    std::vector<std::string> m_resourceTypes;

    int m_selectedTypeIndex = -1;
    int m_selectedInstanceIndex = -1;

    void UpdateResources();
    bool RenderObjectEditor(const std::string& typeName, void* instance);

    // Helper functions
    FBBackend::SDK_CLASS* FindClass(const std::string& name);
    FBBackend::SDK_STRUCT* FindStruct(const std::string& name);
    FBBackend::SDK_ENUM* FindEnum(const std::string& name);

    // Field rendering functions
    bool RenderField(void* basePtr, const std::string& fieldName, const std::string& fieldType,
        unsigned int offset, unsigned int size);

    bool RenderBasicType(void* basePtr, const std::string& fieldName, const std::string& fieldType,
        unsigned int offset);

    bool RenderEnum(void* basePtr, const std::string& fieldName, const std::string& enumType,
        unsigned int offset);

    bool RenderClassPointer(void* basePtr, const std::string& fieldName, const std::string& className,
        unsigned int offset);

    bool RenderStruct(void* basePtr, const std::string& fieldName, const std::string& structName,
        unsigned int offset);

    bool RenderArray(void* basePtr, const std::string& fieldName, const std::string& arrayType,
        unsigned int offset, unsigned int size);

    std::string GetActualClassName(void* instance, const std::string& defaultName);
};