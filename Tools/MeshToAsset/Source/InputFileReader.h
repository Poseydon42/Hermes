#pragma once

#include <optional>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes::Tools
{
	class Mesh;
	class Node;

	class IInputFileReader
	{
		ADD_INTERFACE_CONSTRUCTORS_AND_DESTRUCTOR(IInputFileReader);

	public:
		virtual bool Read(StringView Path) = 0;

		virtual const Node& GetRootNode() const = 0;

		virtual std::optional<const Mesh*> GetMesh(StringView MeshName) const = 0;
	};
}
