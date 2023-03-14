#include "Component.h"

#include "World/Components/DirectionalLightComponent.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/PointLightComponent.h"
#include "World/Components/TransformComponent.h"

namespace Hermes::ComponentIDCounterInternal
{
	ComponentID GNextComponentID = 0;

	ComponentID AllocateComponentID()
	{
		auto Result = GNextComponentID++;
		HERMES_ASSERT_LOG(GNextComponentID > Result, "Reached maximum component count for given component ID data type");

		return Result;
	}
}

namespace Hermes
{
	HERMES_DECLARE_ENGINE_COMPONENT(DirectionalLightComponent);
	HERMES_DECLARE_ENGINE_COMPONENT(MeshComponent);
	HERMES_DECLARE_ENGINE_COMPONENT(PointLightComponent);
	HERMES_DECLARE_ENGINE_COMPONENT(TransformComponent);
}