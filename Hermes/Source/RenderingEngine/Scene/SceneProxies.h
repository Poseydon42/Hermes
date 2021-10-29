#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/MeshBuffer.h"

namespace Hermes
{
	class MeshAsset;

	struct HERMES_API MeshProxy
	{
		Mat4 TransformationMatrix;
		MeshBuffer MeshData;
	};
}
