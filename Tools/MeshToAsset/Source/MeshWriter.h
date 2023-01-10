#pragma once

#include <span>

#include "AssetSystem/MeshAsset.h"
#include "Core/Core.h"

namespace Hermes::Tools
{
	class HERMES_API MeshWriter
	{
	public:
		static bool Write(StringView FileName, std::span<const Vertex> Vertices, std::span<const uint32> Indices);
	};
}
