#pragma once

#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Mesh.h"

namespace Hermes
{
	struct HERMES_API MeshComponent
	{
		AssetHandle<Mesh> Mesh;
		AssetHandle<MaterialInstance> MaterialInstance;
	};
}
