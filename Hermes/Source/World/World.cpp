#include "World.h"

#include "Core/Profiling.h"

namespace Hermes
{
	World::World()
	{
		Entities[InvalidEntity] = 0;
	}

	void World::Update(Scene& Scene, float DeltaTime)
	{
		HERMES_PROFILE_FUNC();
		Scene.Reset();
		for (const auto& System : Systems)
			System->Run(*this, Scene, DeltaTime);
	}

	EntityID World::CreateEntity()
	{
		HERMES_ASSERT(NextEntityID + 1 > NextEntityID);

		auto Entity = NextEntityID++;
		Entities[Entity] = 0;

		return Entity;
	}

	void World::RemoveEntity(EntityID Entity)
	{
		// FIXME: properly destroy all components and reuse the entity ID
		HERMES_ASSERT(Entities.contains(Entity));
		Entities.erase(Entity);
	}

	void World::AddSystem(std::unique_ptr<ISystem> System)
	{
		Systems.push_back(std::move(System));
	}
}
