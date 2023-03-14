#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"

namespace Hermes
{
	struct HERMES_API PointLightComponent
	{
		Vec3 Color;
		float Intensity = 0.0f;
	};
}