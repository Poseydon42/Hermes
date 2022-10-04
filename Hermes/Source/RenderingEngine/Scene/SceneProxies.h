#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/MeshBuffer.h"
#include "RenderingEngine/Scene/Material.h"

namespace Hermes
{
	class MeshAsset;

	struct HERMES_API MeshProxy
	{
		Mat4 TransformationMatrix;
		MeshBuffer MeshData;
		std::shared_ptr<Material> Material;
	};

	struct HERMES_API PointLightProxy
	{
		/* Only first 3 components are meaningful, 4th is added for alignment purposes */
		Vec4 Position;
		Vec4 Color;
	};
}
