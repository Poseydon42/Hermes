#include "LightRenderingSystem.h"

#include "Core/Profiling.h"
#include "World/World.h"
#include "World/Components/DirectionalLightComponent.h"
#include "World/Components/PointLightComponent.h"
#include "World/Components/TransformComponent.h"

namespace Hermes
{
	void LightRenderingSystem::Run(World& World, Scene& Scene, float) const
	{
		HERMES_PROFILE_FUNC();

		for (auto Entity : World.View<PointLightComponent, TransformComponent>())
		{
			const auto* PointLight = World.GetComponent<PointLightComponent>(Entity);
			const auto* Transform = World.GetComponent<TransformComponent>(Entity);

			Scene.GetRootNode().AddChild<PointLightNode>(Transform->Transform, PointLight->Color, PointLight->Intensity);
		}

		for (auto Entity : World.View<DirectionalLightComponent>())
		{
			const auto* Light = World.GetComponent<DirectionalLightComponent>(Entity);

			Scene.GetRootNode().AddChild<DirectionalLightNode>(Transform{}, Light->Direction, Light->Color, Light->Intensity);
		}
	}
}
