#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/BoundingVolume.h"
#include "Math/Math.h"
#include "Math/Transform.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/MeshBuffer.h"

namespace Hermes
{
	struct HERMES_API MeshProxy
	{
		Transform Transform;
		SphereBoundingVolume BoundingVolume;
		std::shared_ptr<MeshBuffer> MeshData;
		std::shared_ptr<MaterialInstance> Material;
	};

	struct HERMES_API PointLightProxy
	{
		/* Only first 3 components are meaningful, 4th is added for alignment purposes */
		Vec4 Position;
		Vec4 Color;
	};

	struct HERMES_API DirectionalLightProxy
	{
		Vec3 Direction;
		Vec3 Color;
		float Intensity = 0.0f;
	};
}
