#include "MeshRenderingSystem.h"

#include "Core/Profiling.h"
#include "RenderingEngine/Scene/Scene.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/TransformComponent.h"
#include "World/World.h"

namespace Hermes
{
	void MeshRenderingSystem::Run(World& World, Scene& Scene, float) const
	{
		HERMES_PROFILE_FUNC();

		auto RenderableEntities = World.View<MeshComponent, TransformComponent>();
		for (auto Entity : RenderableEntities)
		{
			const auto* Mesh = World.GetComponent<MeshComponent>(Entity);
			const auto* Transform = World.GetComponent<TransformComponent>(Entity);

			Scene.GetRootNode().AddChild<MeshNode>(Transform->Transform, Mesh->Mesh, Mesh->Material);
		}
	}
}
