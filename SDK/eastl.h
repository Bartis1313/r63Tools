#pragma once

#include <cstdint>
#include <cstring>

// https://github.com/electronicarts/EASTL
namespace eastl
{
	template <typename T>
	class vector
	{
	public:
		size_t size() const
		{
			return (reinterpret_cast<uintptr_t>(m_End) - reinterpret_cast<uintptr_t>(m_FirstElement)) / sizeof(T);
		}

		T at(size_t index)
		{
			return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(m_FirstElement) + index * sizeof(T));
		}

		T operator[](size_t index)
		{
			return at(index);
		}

		T* begin()
		{
			return m_FirstElement;
		}
		const T* begin() const
		{
			return m_FirstElement;
		}

		T* end()
		{
			return m_End;
		}

		const T* end() const
		{
			return m_End;
		}

		bool empty()
		{
			return size() == 0;
		}

		size_t capacity() const
		{
			return reinterpret_cast<uintptr_t>(m_ArrayBound) - reinterpret_cast<uintptr_t>(m_FirstElement) / sizeof(T);
		}

	private:
		T* m_FirstElement = nullptr;
		T* m_End = nullptr;
		T* m_ArrayBound = nullptr;
		void* vftable = nullptr;
	};

	template< class T, int32_t T_Count, uint32_t T_PadSize>
	class fixed_vector
	{
	public:
		T* m_firstElement;
		T* m_lastElement;
		T* m_arrayBound;
		T m_data[T_Count];

	public:
		fixed_vector(void)
		{
			m_firstElement = reinterpret_cast<T*>(m_data);
			m_lastElement = reinterpret_cast<T*>(m_data);
			m_arrayBound = reinterpret_cast<T*>(&m_data[T_Count]);
		}

		void push_back(T* const value)
		{
			if (m_lastElement > m_arrayBound)
			{
				return;
			}
			*m_lastElement = *value;
			m_lastElement = m_lastElement + 1;
		};

		void clear(void)
		{
			m_firstElement = m_data;
			m_lastElement = m_data;
		}

		uint32_t size() const
		{
			return m_lastElement - m_firstElement;
		}

		T at(size_t nIndex)
		{
			return m_firstElement[nIndex];
		}

		T operator [](size_t index)
		{
			return at(index);
		}
	};

	struct string
	{
		char* begin;
		char const* end;
		char const* capacity;
		void* allocator_arena;

		void assign(const char* newStr)
		{
			typedef void* (__fastcall* tfgbAssign)(string* a1, char const* a2, char const* a3);
			tfgbAssign fbAssign = (tfgbAssign)0x14000F6B0;

			fbAssign(this, newStr, newStr + strlen(newStr));
		}

		auto c_str()
		{
			return begin;
		}
	};
}