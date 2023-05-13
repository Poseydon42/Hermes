#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/Scene/Scene.h"
#include "World/Component.h"
#include "World/Entity.h"
#include "World/System.h"

namespace Hermes
{
	/*
	 * NOTE: this all is a very *very* basic & naive implementation
	 * The performance is probably a lot worse than a simple OOP approach
	 * This *will* change in the future by a lot, my goal here is to just
	 * have something with more or less stable API to prototype the rest
	 * of the engine & return to this thing later to improve it (by a lot).
	 */
	class HERMES_API World
	{
		MAKE_NON_COPYABLE(World)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(World)
		ADD_DEFAULT_DESTRUCTOR(World)

	public:
		World();

		void Update(Scene& Scene, float DeltaTime);

		EntityID CreateEntity();

		void RemoveEntity(EntityID Entity);

		template<typename ComponentType>
		ComponentType& AddComponent(EntityID Entity);

		template<typename ComponentType>
		void RemoveComponent(EntityID Entity);

		template<typename ComponentType>
		ComponentType* GetComponent(EntityID Entity);
		template<typename ComponentType>
		const ComponentType* GetComponent(EntityID Entity) const;

		template<typename... ComponentTypes>
		std::vector<EntityID> View() const;

		void AddSystem(std::unique_ptr<ISystem> System);

	private:
		template<typename ComponentType>
		std::vector<ComponentType>& GetComponentContainer();

		/*
		 * NOTE: this ugly and probably unsafe code is here because
		 * we need to make sure that the component containers are located
		 * strictly in the engine executable and that they're not duplicated
		 * in the game DLL. To do so, we need to break a chain of template
		 * functions with something strictly non-template, like dynamic polymorphism.
		 *
		 * FIXME
		 */
		struct AbstractComponentContainer
		{
			virtual ~AbstractComponentContainer() = default;

			/*
			 * NOTE: will return std::vector<ComponentType>*
			 */
			virtual void* GetContainer() = 0;
		};
		template<typename ComponentType>
		struct ComponentContainer : AbstractComponentContainer
		{
		public:
			virtual void* GetContainer() override;

		private:
			std::vector<ComponentType> Container;
		};

		std::unordered_map<ComponentID, std::unique_ptr<AbstractComponentContainer>> ComponentContainers;

		EntityID NextEntityID = 1;
		std::unordered_map<EntityID, ComponentBitmask> Entities;

		std::vector<std::unique_ptr<ISystem>> Systems;
	};

	template<typename ComponentType>
	ComponentType& World::AddComponent(EntityID Entity)
	{
		HERMES_ASSERT(Entities.contains(Entity));
		HERMES_ASSERT(Entity != InvalidEntity);

		auto& Container = GetComponentContainer<ComponentType>();
		if (Container.size() <= static_cast<size_t>(Entity))
			Container.resize(static_cast<size_t>(Entity) + 1);

		Entities[Entity] |= GetComponentBitmask<ComponentType>();

		Container[Entity] = {};

		return Container[Entity];
	}

	template<typename ComponentType>
	void World::RemoveComponent(EntityID Entity)
	{
		// FIXME: properly destroy components (call destructor)
		HERMES_ASSERT(Entities.contains(Entity));
		HERMES_ASSERT(Entity != InvalidEntity);

		Entities[Entity] &= ~GetComponentBitmask<ComponentType>();
	}

	template<typename ComponentType>
	ComponentType* World::GetComponent(EntityID Entity)
	{
		HERMES_ASSERT(Entities.contains(Entity));
		HERMES_ASSERT(Entity != InvalidEntity);

		if ((Entities[Entity] & GetComponentBitmask<ComponentType>()) == 0)
			return nullptr;

		auto& Container = GetComponentContainer<ComponentType>();
		auto& Result = Container[Entity];

		return &Result;
	}

	template<typename ComponentType>
	const ComponentType* World::GetComponent(EntityID Entity) const
	{
		return const_cast<World*>(this)->GetComponent<ComponentType>(Entity);
	}

	template<typename... ComponentTypes>
	std::vector<EntityID> World::View() const
	{
		auto Bitmask = GetComponentPackBitmask<ComponentTypes...>();

		std::vector<EntityID> Result;
		for (const auto& Entity : Entities)
		{
			if ((Entity.second & Bitmask) == Bitmask)
				Result.push_back(Entity.first);
		}

		return Result;
	}

	template<typename ComponentType>
	std::vector<ComponentType>& World::GetComponentContainer()
	{
		auto ComponentID = GetComponentID<ComponentType>();

		if (!ComponentContainers.contains(ComponentID))
			ComponentContainers[ComponentID] = std::make_unique<ComponentContainer<ComponentType>>();

		return *static_cast<std::vector<ComponentType>*>(ComponentContainers[ComponentID]->GetContainer());
	}

	template<typename ComponentType>
	void* World::ComponentContainer<ComponentType>::GetContainer()
	{
		return &Container;
	}
}
