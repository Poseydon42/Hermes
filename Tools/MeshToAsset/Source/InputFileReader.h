#pragma once

#include <span>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "AssetSystem/MeshAsset.h"

namespace Hermes::Tools
{
	class IInputFileReader
	{
		ADD_INTERFACE_CONSTRUCTORS_AND_DESTRUCTOR(IInputFileReader);

	public:
		virtual bool Read(StringView Path) = 0;

		virtual bool HasTangents() const = 0;

		virtual bool IsTriangulated() const = 0;

		/*
		 * Returns a list of unique vertices that are present in the mesh.
		 *
		 * NOTE: not all fields of vertex struct must be populated as not all source formats support them (e.g. tangents).
		 */
		virtual std::span<const Vertex> GetVertices() const = 0;

		/*
		 * Returns a list of indices. Individual faces are separated by -1. Faces are not guaranteed to be triangular (see IsTriangulated())
		 */
		virtual std::span<const uint32> GetIndices() const = 0;
	};
}
