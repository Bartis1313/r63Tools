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
			return *(GameWorld**)OFFSET_GAMEWORLD;
		}
	};

	struct OnlineId
	{
		uint64_t m_nativeData; // 0x0000
		char m_name[0x11]; // 0x0008
		char _0x0019[0x7]; // 0x0019
	};

	class UnknownContextHolder
	{
	public:
		char pad_0000[3128]; //0x0000
		/*class N00001D42**/ __int64 chatMNGR; //0x0C38
		char pad_0C40[1032]; //0x0C40
	};

#ifdef _WIN64
	class ServerGameContext
	{
	public:
		char pad_0000[8]; //0x0000
		class UnknownContextHolder* unkHolder; //0x0008
		char pad_0010[16]; //0x0010
		class N0000192C* N000000C5; //0x0020
		class ServerLevel* m_serverLevel; //0x0028
		char pad_0030[48]; //0x0030
		class ServerPlayerManager* m_serverPlayerManager; //0x0060
		class ServerPeer* m_serverPeer; //0x0068
		char pad_0070[120]; //0x0070
		char* m_LevelName; //0x00E8
		char pad_00F0[24]; //0x00F0
		char* m_LevelMode; //0x0108
		static ServerGameContext* GetInstance()
		{
			return *(ServerGameContext**)OFFSET_GAMECONTEXT;
		}
	};
#elif _WIN32
	class ServerGameContext
	{
	public:
		char pad_0000[16]; //0x0000
		class ServerLevel* m_serverLevel; //0x0010
		char pad_0014[28]; //0x0014
		class ServerPlayerManager* m_serverPlayerManager; // 0x0034
		class ServerPeer* m_serverPeer;// 0x0038
		static ServerGameContext* GetInstance()
		{
			return *(ServerGameContext**)OFFSET_GAMECONTEXT;
		}
	};
#endif

	class ServerPlayer
	{
	public:
		virtual ~ServerPlayer();
		virtual void* getCharacterEntity();
		virtual void* getCharacterData();
		virtual void* getEntryComponent(); // serverconnection 0x50
		virtual bool isInVehicle();
		virtual uint32_t getId();
		virtual bool hasAsset(void*, bool);
		virtual bool getAssets(void**);
		virtual bool isUnlockedAsset(void*);

#ifdef _WIN64
		__int64 m_playerData; // 0x0008
#elif _WIN32
		int unkStart; //0x0004
		void* m_playerData; // 0x0008 
#endif
#ifdef _WIN64
		char _0x0010[8]; // 0x0010
#elif _WIN32
		char pad[4];
#endif
		eastl::string m_Name; // 0x0018
		OnlineId m_onlineId; // 0x0038
	};

	class ServerPlayerManager
	{
	public:
		virtual ~ServerPlayerManager();
		virtual eastl::vector<ServerPlayer*>* getPlayers();
#ifdef _WIN64
		virtual eastl::vector<ServerPlayer*>* getSpectators();
#endif
	};

#ifdef _WIN64
	struct EntityIterableLink
	{
		EntityIterableLink* next;
		EntityIterableLink* prev;
	};

	template <typename T> class EntityIterator;
	template <typename T> class EntityList;

	template <typename T>
	class EntityList 
	{
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
	class EntityIterator 
	{
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
#elif _WIN32
	enum Realm
	{
		Realm_Client,
		Realm_Server,
		Realm_ClientAndServer,
		Realm_None,
		Realm_Pipeline,
	};
	class EntityCreator
	{
	public:
		enum RayCastTest
		{
			RCTDetailed = 0,
			RCTCollision
		};
		LPVOID vftable;						// 0x00
		EntityCreator* m_previousCreator;	// 0x04
		EntityCreator* m_nextCreator;		// 0x08
		Realm m_realm;						// 0x0C
		INT m_linked;						// 0x10
	}; // 0x14

	class EntityCollectionSegment;

	class EntityCollection
	{
	public:
		class EntityCollectionSegment* firstSegment; // 0x0
		class EntityCreator* creator; // 0x4
	};

	class EntityCollectionSegment
	{
	public:
		eastl::vector<void*> m_Collection;
		void* m_subLevel; // 0x10
		EntityCollectionSegment* m_prev; // 0x14
		EntityCollectionSegment* m_next; // 0x18
		DWORD m_iterableSize; // 0x1C
		DWORD m_collectionIndex; // 0x20
	};

	class EntityWorld
	{
	public:
		class EntityIterator
		{
		public:
			eastl::vector<EntityCollection>* m_collections;    //offset = 0x0, length = 0x4
			EntityCollectionSegment* m_currentSegment;    //offset = 0x4, length = 0x4
			unsigned int m_collectionIndexIt;    //offset = 0x8, length = 0x4
			unsigned int m_collectionIndexEnd;    //offset = 0xC, length = 0x4
			unsigned int m_entityIndexIt;    //offset = 0x10, length = 0x4
			unsigned int m_entityIndexEnd;    //offset = 0x14, length = 0x4
			bool m_onlyIncludeIterable;    //offset = 0x18, length = 0x1
			char pad_19[3];
		};

		void kindOfQuery(ClassInfo* classInfo, EntityIterator* result, bool onlyIncludeIterable = true)
		{
			typedef void(__thiscall* kindOfQuery_t)(void*, ClassInfo*, EntityIterator*, bool);
			kindOfQuery_t m_kindOfQuery = (kindOfQuery_t)OFFSET_GETENTITYLIST;

			m_kindOfQuery(this, classInfo, result, onlyIncludeIterable);
		}
	};

	template<typename T> struct EntityList : public EntityWorld::EntityIterator
	{
		EntityList() { }
		EntityList(fb::ClassInfo* classInfo, bool onlyIncludeIterable = true)
		{
			EntityWorld* manager = (EntityWorld*)GameWorld::GetInstance();
			if (!manager)
				return;

			manager->kindOfQuery(classInfo, this, onlyIncludeIterable);
		}

		struct Iterator
		{
			EntityIterator iter;

			Iterator(const EntityIterator& base) : iter(base) { }

			bool operator!=(const Iterator& other) const
			{
				return iter.m_collectionIndexIt != other.iter.m_collectionIndexIt ||
					iter.m_entityIndexIt != other.iter.m_entityIndexIt;
			}

			T* operator*() const
			{
				if (iter.m_currentSegment && iter.m_entityIndexIt < iter.m_entityIndexEnd)
				{
					return static_cast<T*>(iter.m_currentSegment->m_Collection[iter.m_entityIndexIt]);
				}

				return nullptr;
			}

			T* operator->() const
			{
				return operator*();
			}

			Iterator& operator++()
			{
				iter.m_entityIndexIt++;
				if (iter.m_entityIndexIt >= iter.m_entityIndexEnd)
				{
					iter.m_collectionIndexIt++;
				}
				return *this;
			}
		};

		Iterator begin() const { return Iterator(*this); }
		Iterator end() const
		{
			EntityIterator it = *this;
			it.m_collectionIndexIt = it.m_collectionIndexEnd;
			it.m_entityIndexIt = it.m_entityIndexEnd;
			return Iterator(it);
		}
	};
#endif

	class ResourceManager
	{
	public:

		class Compartment
		{
		public:
#ifdef _WIN64
			char pad[0x128];
#elif _WIN32
			char pad_0000[84]; //0x0000
			char* m_domain; //0x0054
			char pad_0058[144]; //0x0058
#endif
			eastl::vector<TypeInfoObject*> m_objects;
		};

		static ResourceManager* GetInstance()
		{
			return *(ResourceManager**)RESOURCE_MANAGER;
		}

	public:
		unsigned volatile int m_bundleLoadInProgress;
		// BD ? ? ? ? 66 0F 1F 44 00 ? 48 8B 1F 48 85 DB 74 71 48 8B CB E8 ? ? ? ? 48 8B 93 ? ? ? ? 4C 8B 83 ? ? ? ? 4C 2B C2 r63
		// BD ? ? ? ? 8B 33 85 F6 74 67 8B CE  r38
		// ebp stores size
		/*
		* lea     rdi, [rcx+8]
.text:00000001407F2305 BD BD 0B 00 00                                                  mov     ebp, 0BBDh
.text:00000001407F230A 66 0F 1F 44 00 00                                               nop     word ptr [rax+rax+00h]
		*/
#ifdef  _WIN64
		static constexpr int COMPARTMENT_COUNT = 3005;
#elif _WIN32
		static constexpr int COMPARTMENT_COUNT = 205;
#endif
		static constexpr int compartmenstSize() { return COMPARTMENT_COUNT; }

		Compartment* m_compartments[COMPARTMENT_COUNT];
	};
}