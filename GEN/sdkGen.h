#pragma once

#include "sdkTypes.h"
#include <unordered_map>

namespace FBBackend
{
    class SDK_GENERATOR
    {
    public:
        SDK_GENERATOR();
        virtual ~SDK_GENERATOR();

        void Destroy();
        void Register(fb::TypeInfo* typeInfo);
        void Analyze();

        std::vector<fb::TypeInfo*>* m_typeInfos;
        std::vector<fb::ClassInfo*>* m_classInfos;
        std::vector<fb::EnumFieldInfo*>* m_enumInfos;
        std::vector<fb::ValueTypeInfo*>* m_valueInfos;
        std::vector<PSDK_CLASS>* m_sdkClasses;
        std::vector<PSDK_ENUM>* m_sdkEnums;
        std::vector<PSDK_STRUCT>* m_sdkStructs;

        bool m_generateStaticMembers;
        bool m_generateAboutTypeinfo;

    private:
        void AnalyzeClasses();
        void AnalyzeEnums();
        void AnalyzeStructs();
    public:
        void setStaticMembers(bool state) { m_generateStaticMembers = state; }
        void setStaticComments(bool state) { m_generateAboutTypeinfo = state; }
        // only one, there is no need
        void GenerateDeclarations(std::vector<std::string>& result);
        // below functions Generate... second arg contains helper for menu to operate by names, and display it
        void GenerateClasses(std::vector<std::string>& result, std::vector<std::string>& classesNames);
        void GenerateEnums(std::vector<std::string>& result, std::vector<std::string>& enumsNames);
        void GenerateStructs(std::vector<std::string>& result, std::vector<std::string>& structsNames);
    private:
        std::string GetTypeName(fb::TypeInfo* typeInfo);
        unsigned int GetTypeSize(fb::TypeInfo* typeInfo);
        void GetDependenciesForStructMember(fb::TypeInfo* typeInfo, std::vector<PSDK_STRUCT>& dependencies);
    };

    typedef SDK_GENERATOR* PSDK_GENERATOR;
}
