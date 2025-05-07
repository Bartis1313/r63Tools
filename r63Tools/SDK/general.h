#pragma once

#include <cstdint>
#include <cassert>

namespace fb
{
    template<typename T> class WeakToken
    {
    public:
        T* m_RealPtr; //0x0000
    };
    //Size=0x0008

    template<typename T> class ConstWeakPtr
    {
    public:
        WeakToken<T>* m_Token; //0x0000

        T* Get()
        {
            if (m_Token && m_Token->m_RealPtr)
            {
                uintptr_t realPtr = (uintptr_t)m_Token->m_RealPtr;
                return (T*)(realPtr - sizeof(uintptr_t));
            }
            return NULL;
        }
    };
    //Size=0x0008

    template< class T > class WeakPtr
    {
    public:
        T** m_ptr;

    public:
        T* GetData()
        {
            __try
            {
                if (!m_ptr)
                    return NULL;

                if (!*m_ptr)
                    return NULL;

                T* ptr = *m_ptr;

                return (T*)((uintptr_t)ptr - 0x8);
            }
            __except (1)
            {
                return NULL;
            }
        }
    };
    //Size=0x0008

    struct Guid
    {
        unsigned long	m_Data1;	//0x0000
        unsigned short	m_Data2;	//0x0004
        unsigned short	m_Data3;	//0x0006
        unsigned char	m_Data4[8];	//0x0008
    };
    //Size=0x0010

    struct Color32
    {
        union
        {
            struct
            {
                unsigned char m_R;	//0x0000
                unsigned char m_G;	//0x0001
                unsigned char m_B;	//0x0002
                unsigned char m_A;	//0x0003
            };

            unsigned int m_Data;	//0x0000
        };

        Color32(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1);
        Color32(unsigned int col);
    };
    //Size=0x0004

    template<class T> class Tuple2
    {
    public:
        T m_Element1;
        T m_Element2;

        Tuple2(T element1, T element2)
        {
            m_Element1 = element1;
            m_Element2 = element2;
        }
    };
    //Size=2*T

    struct ArrayHeader
    {
        uint32_t unk;
        uint32_t size;
    };

    template <typename T>
    class Array
    {
    public:
        T* m_firstElement;

    public:
        Array() : m_firstElement(nullptr)
        {
        }

        ArrayHeader* GetHeader() const
        {
            if (!m_firstElement)
                return nullptr;

            return reinterpret_cast<ArrayHeader*>(
                reinterpret_cast<char*>(m_firstElement) - sizeof(ArrayHeader)
                );
        }

        // Access element at given index
        T& At(uint32_t nIndex)
        {
            return *(T*)((intptr_t)m_firstElement + (nIndex * sizeof(T)));
        }

        uint32_t size() const
        {
            ArrayHeader* header = GetHeader();
            if (!header)
                return 0;

            return header->size;
        }

        T* begin()
        {
            return m_firstElement;
        }

        T* end()
        {
            return m_firstElement + size();
        }

        T& operator[](uint32_t index) { return At(index); }
    };

    class ArrayBase
    {
    public:
        void* m_firstElement;

        ArrayHeader* GetHeader() const
        {
            if (!m_firstElement)
                return nullptr;

            return reinterpret_cast<ArrayHeader*>(
                reinterpret_cast<char*>(m_firstElement) - sizeof(ArrayHeader)
                );
        }

        uint32_t size() const
        {
            ArrayHeader* header = GetHeader();
            if (!header)
                return 0;

            return header->size;
        }
    };
}