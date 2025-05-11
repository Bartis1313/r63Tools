#pragma once

#include <cstdint>

#include "eastl.h"
#include "typeinfo.h"
#include "offsets.h"
#include "checks.h"

namespace fb
{
	class GameWorld
	{
	public:
		static GameWorld* GetInstance()
		{
			return *(GameWorld**)0x142C71AD0;
		}
	};

	struct EntityIterableLink
	{
		EntityIterableLink* next;
		EntityIterableLink* prev;
	};

	template <typename T> class EntityIterator;
	template <typename T> class EntityList;

	template <typename T>
	class EntityList {
	public:
		using iterator = EntityIterator<T>;

	private:
		iterator m_first;
	public:
		inline iterator begin() const { return m_first; }
		inline iterator end() const { return {}; }

		EntityList(ClassInfo* typeInfo, intptr_t offset = 0x40)
		{
			auto gameWorld = GameWorld::GetInstance();
			if (!gameWorld) return;

			EntityIterableLink* firstLink = getFirstIterableLink(typeInfo, gameWorld);
			m_first = iterator(firstLink, offset);
		}

		size_t size() const
		{
			size_t count = 0;
			auto iter = m_first;
			while (iter != end())
			{
				count++;
				++iter;
			}
			return count;
		}

	private:
		static EntityIterableLink* getFirstIterableLink(ClassInfo* classInfo, GameWorld* gameWorld)
		{
			EntityIterableLink* (__fastcall * native)(ClassInfo*, GameWorld*) =
				reinterpret_cast<EntityIterableLink * (__fastcall*)(ClassInfo*, GameWorld*)>(OFFSET_GETENTITYLIST);

			return native(classInfo, gameWorld);
		}
	};

	template <typename T>
	class EntityIterator {
	private:
		EntityIterableLink* m_link;
		intptr_t m_offset;

	public:
		inline EntityIterator() :
			m_link{ nullptr }, m_offset{ 0 }
		{
		}
		inline EntityIterator(EntityIterableLink* link, intptr_t offset)
			: m_link{ link }, m_offset{ offset }
		{
		}

		inline EntityIterator& operator++()
		{
			if (m_link)
				m_link = m_link->next;
			return *this;
		}

		inline EntityIterator operator++(int)
		{
			EntityIterator tmp = *this;
			if (m_link)
				m_link = m_link->next;
			return tmp;
		}

		inline bool operator!=(const EntityIterator& rhs) const
		{
			return m_link != rhs.m_link;
		}

		inline bool operator==(const EntityIterator& rhs) const
		{
			return m_link == rhs.m_link;
		}

		inline T* operator*() const
		{
			if (!m_link)
				return nullptr;

			uintptr_t entityAddr = reinterpret_cast<uintptr_t>(m_link) - m_offset;
			return reinterpret_cast<T*>(entityAddr);
		}

		inline T* operator->() const
		{
			return operator*();
		}

		inline EntityIterableLink* getLink() const
		{
			return m_link;
		}
	};

	class ResourceManager
	{
	public:

		class Compartment
		{
		public:
			char pad[0x128];
			eastl::vector<TypeInfoObject*> m_objects;
		};

		static ResourceManager* GetInstance()
		{
			return *(ResourceManager**)RESOURCE_MANAGER;
		}

	public:
		unsigned volatile int m_bundleLoadInProgress;
		// BD ? ? ? ? 66 0F 1F 44 00 ? 48 8B 1F 48 85 DB 74 71 48 8B CB E8 ? ? ? ? 48 8B 93 ? ? ? ? 4C 8B 83 ? ? ? ? 4C 2B C2
		// ebp stores size
		/*
		* lea     rdi, [rcx+8]
.text:00000001407F2305 BD BD 0B 00 00                                                  mov     ebp, 0BBDh
.text:00000001407F230A 66 0F 1F 44 00 00                                               nop     word ptr [rax+rax+00h]
		*/
		constexpr int compartmenstSize() { return 3005; }
		Compartment* m_compartments[3005];
	};
}