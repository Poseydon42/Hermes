#pragma once

#include "AssetSystem/AssetCache.h"
#include "Core/Core.h"

namespace Hermes
{
	struct HERMES_API MeshComponent
	{
		AssetHandle Mesh;
		AssetHandle MaterialInstance;
	};
}
