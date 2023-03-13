#include "World.h"

#include "Core/Profiling.h"

namespace Hermes
{
	World::World()
	{
		Entities[InvalidEntity] = 0;
	}

	void World::Update(float DeltaTime)
	{
		HERMES_PROFILE_FUNC();
		Scene.Reset();
		for (const auto& System : Systems)
			System->Run(*this, Scene, DeltaTime);
	}

	const Scene& World::GetScene() const
	{
		return Scene;
	}

	EntityID World::CreateEntity()
	{
		HERMES_ASSERT(NextEntityID + 1 > NextEntityID);

		auto Entity = NextEntityID++;
		Entities[Entity] = 0;

		return Entity;
	}

	void World::AddSystem(std::unique_ptr<ISystem> System)
	{
		Systems.push_back(std::move(System));
	}
}
