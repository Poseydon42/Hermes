#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"

namespace Hermes
{
	struct HERMES_API DirectionalLightComponent
	{
		Vec3 Direction;
		Vec3 Color;
		float Intensity = 0.0f;
	};
}
