#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"
#include "World/Component.h"

namespace Hermes
{
	struct HERMES_API DirectionalLightComponent
	{
		Vec3 Direction;
		Vec3 Color;
		float Intensity = 0.0f;
	};

	HERMES_DECLARE_ENGINE_COMPONENT(DirectionalLightComponent);
}
