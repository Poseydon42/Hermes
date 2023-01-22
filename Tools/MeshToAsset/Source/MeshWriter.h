#pragma once

#include <span>

#include "Core/Core.h"

namespace Hermes::Tools
{
	class Mesh;

	class HERMES_API MeshWriter
	{
	public:
		static bool Write(StringView FileName, const Mesh& Mesh);
	};
}
