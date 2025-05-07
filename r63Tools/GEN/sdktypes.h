#pragma once

#include <string>
#include <vector>
#include "../SDK/typeinfo.h"

enum class SDKType : int
{
    E_TYPE_CLASS = 1,
    E_TYPE_STRUCT = 2,
    E_TYPE_ENUM = 3
};

namespace FBBackend
{
    struct SDK_CLASS
    {
        SDKType m_sdkType;

        struct SDK_CLASS_MEMBER
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            std::string m_type;
            unsigned int m_offset;
            unsigned int m_size;
            bool m_isPad;

            inline bool operator<(const SDK_CLASS_MEMBER& other);
        };
        typedef SDK_CLASS_MEMBER* PSDK_CLASS_MEMBER;

        fb::ClassInfo* m_rawClassInfo;
        fb::DataContainer* m_rawDefaultInstance;

        SDK_CLASS* m_parent;
        std::string m_name;
        unsigned int m_classId;
        unsigned int m_runtimeId;
        unsigned int m_size;
        unsigned short m_alignment;

        std::vector<SDK_CLASS_MEMBER> m_sdkClassMembers;
        std::vector<SDK_CLASS*> m_sdkClassDependencies;
    };
    typedef SDK_CLASS* PSDK_CLASS;


    struct FindByRawClassInfo
    {
        inline explicit FindByRawClassInfo(fb::ClassInfo* classInfo);

        fb::ClassInfo* m_classInfo;

        inline bool operator()(const PSDK_CLASS& other) const;
    };
}

namespace FBBackend
{
    struct SDK_ENUM
    {
        SDKType m_sdkType;

        struct SDK_ENUM_FIELD
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            unsigned int m_offset;

            inline bool operator<(const SDK_ENUM_FIELD& other);
        };
        typedef SDK_ENUM_FIELD* PSDK_ENUM_FIELD;

        fb::EnumFieldInfo* m_rawEnumInfo;

        std::string m_name;
        unsigned int m_runtimeId;

        std::vector<SDK_ENUM_FIELD> m_sdkEnumFields;
    };
    typedef SDK_ENUM* PSDK_ENUM;
}

namespace FBBackend
{
    struct SDK_STRUCT
    {
        SDKType m_sdkType;

        struct SDK_STRUCT_MEMBER
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            std::string m_type;
            unsigned int m_offset;
            unsigned int m_size;
            bool m_isPad;

            inline bool operator<(const SDK_STRUCT_MEMBER& other);
        };
        typedef SDK_STRUCT_MEMBER* PSDK_STRUCT_MEMBER;

        fb::ValueTypeInfo* m_rawValueInfo;

        std::string m_name;
        unsigned int m_runtimeId;
        unsigned int m_size;

        std::vector<SDK_STRUCT_MEMBER> m_sdkStructMembers;
        std::vector<SDK_STRUCT*> m_sdkStructDependencies;
    };
    typedef SDK_STRUCT* PSDK_STRUCT;


    struct FindByRawValueInfo
    {
        inline explicit FindByRawValueInfo(fb::ValueTypeInfo* valueInfo);

        fb::ValueTypeInfo* m_valueInfo;

        inline bool operator()(const PSDK_STRUCT& other) const;
    };
}

namespace FBBackend
{
    bool SDK_CLASS::SDK_CLASS_MEMBER::operator<(const SDK_CLASS_MEMBER& other)
    {
        return m_offset < other.m_offset;
    }


    FindByRawClassInfo::FindByRawClassInfo(fb::ClassInfo* classInfo)
    {
        m_classInfo = classInfo;
    }


    bool FindByRawClassInfo::operator()(const PSDK_CLASS& other) const
    {
        return m_classInfo == other->m_rawClassInfo;
    }
}

namespace FBBackend
{
    bool SDK_ENUM::SDK_ENUM_FIELD::operator<(const SDK_ENUM_FIELD& other)
    {
        return m_offset < other.m_offset;
    }
}

namespace FBBackend
{
    bool SDK_STRUCT::SDK_STRUCT_MEMBER::operator<(const SDK_STRUCT_MEMBER& other)
    {
        return m_offset < other.m_offset;
    }


    inline bool SortByRuntimeId(PSDK_STRUCT left, PSDK_STRUCT right)
    {
        return left->m_rawValueInfo->m_RuntimeId < right->m_rawValueInfo->m_RuntimeId;
    }


    FindByRawValueInfo::FindByRawValueInfo(fb::ValueTypeInfo* valueInfo)
    {
        m_valueInfo = valueInfo;
    }


    bool FindByRawValueInfo::operator()(const PSDK_STRUCT& other) const
    {
        return m_valueInfo == other->m_rawValueInfo;
    }
}