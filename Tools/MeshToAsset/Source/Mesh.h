#pragma once

#include <optional>

#include "AssetSystem/MeshAsset.h"
#include "Core/Core.h"

namespace Hermes::Tools
{
	class HERMES_API Mesh
	{
	public:
		Mesh(String InName, std::vector<Vertex> InVertices, std::vector<uint32> InIndices, bool InAreTangentsComputed);

		/*
		 * Returns true if the mesh has tangents defined for each vertex
		 */
		bool HasTangents() const;

		/*
		 * Returns true if all faces of the mesh are triangles
		 */
		bool IsTriangulated() const;

		/*
		 * Inverts the order of vertices in each face
		 */
		void FlipVertexOrder();

		/*
		 * Computes tangents for each vertex in the given array and stores them. Expects the mesh to be triangulated
		 */
		void ComputeTangents();

		StringView GetName() const;
		const std::vector<Vertex>& GetVertices() const;
		const std::vector<uint32>& GetIndices() const;

	private:
		String Name;

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		bool CheckIfIsTriangulated() const;

		bool HasBeenTriangulated;
		bool AreTangentsComputed = false;
	};
}
