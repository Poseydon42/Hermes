#pragma once

#include "AssetSystem/AssetCache.h"
#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialInstance.h"

namespace Hermes
{
	struct HERMES_API MeshComponent
	{
		AssetHandle Mesh;
		const MaterialInstance* Material = nullptr; // FIXME: this is unsafe
	};
}
