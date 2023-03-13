#pragma once

#include "AssetSystem/AssetCache.h"
#include "Core/Core.h"
#include "World/Component.h"

namespace Hermes
{
	struct HERMES_API MeshComponent
	{
		AssetHandle Mesh;
		const MaterialInstance* Material = nullptr; // FIXME: this is unsafe
	};

	HERMES_DECLARE_COMPONENT(MeshComponent);
}
