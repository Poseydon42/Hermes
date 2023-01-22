#include "Mesh.h"

#include <utility>

namespace Hermes::Tools
{
	/*
	 * Calculates tangent vector of a single vertex V1 that is inside a triangle formed
	 * by vertices V1, V2 and V3 in counterclockwise order
	 * NOTE : this calculation does not care about smooth edges and shared vertices - it's caller
	 *        responsibility to average tangents of a shared vertex
	 */
	static Vec3 CalculateSingleTangent(const Vertex& V1, const Vertex& V2, const Vertex& V3)
	{
		const auto P1 = V1.Position;
		const auto P2 = V2.Position;
		const auto P3 = V3.Position;

		const auto T1 = V1.TextureCoordinates;
		const auto T2 = V2.TextureCoordinates;
		const auto T3 = V3.TextureCoordinates;

		Vec3 Edge12 = { P2.X - P1.X, P2.Y - P1.Y, P2.Z - P1.Z };
		Vec3 Edge13 = { P3.X - P1.X, P3.Y - P1.Y, P3.Z - P1.Z };
		Vec2 DeltaUV12 = { T2.X - T1.X, T2.Y - T1.Y };
		Vec2 DeltaUV13 = { T3.X - T1.X, T3.Y - T1.Y };

		/*
		 * Formula derivation:
		 * Let T denote tangent vector and B denote bitangent. Because tangent and bitangent are collinear with
		 * the U and V axes we can express edges of the triangle as following:
		 * E12 = DeltaUV12.X * T + DeltaUV12.Y * B
		 * E13 = DeltaUV13.X * T + DeltaUV13.Y * B
		 *
		 * Because we only want to find the tangent vector let's express B using T the first equation:
		 * B = (E12 - DeltaUV12.X * T) / DeltaUV12.Y
		 *
		 * Substitute it into the second equation:
		 * E13 = DeltaUV13.X * T + DeltaUV13.Y * (E12 - DeltaUV12.X * T) / DeltaUV12.Y
		 *
		 * Solve for T:
		 * DeltaUV12.Y * E13 = DeltaUV12.Y * DeltaUV13.X * T + DeltaUV13.Y * (E12 - DeltaUV12.X * T)
		 * T * (DeltaUV12.Y * DeltaUV13.X - DeltaUV12.X * DeltaUV13.Y) = DeltaUV12.Y * E13 - DeltaUV13.Y * E12
		 * T = (DeltaUV12.Y * E13 - DeltaUV13.Y * E12) / (DeltaUV12.Y * DeltaUV13.X - DeltaUV12.X * DeltaUV13.Y)
		 */

		Vec3 Numerator = Edge13 * DeltaUV12.Y - Edge12 * DeltaUV13.Y;
		float Denominator = DeltaUV12.Y * DeltaUV13.X - DeltaUV12.X * DeltaUV13.Y;
		// Use default UV direction when denominator is 0
		if (Denominator <= 0.000001f)
		{
			DeltaUV12 = { 0.0f, 1.0f };
			DeltaUV13 = { 1.0f, 0.0f };
			Denominator = 1.0f;
		}

		Vec3 Result = (Numerator / Denominator).SafeNormalize();

		return Result;
	}

	Mesh::Mesh(String InName, std::vector<Vertex> InVertices, std::vector<uint32> InIndices, bool InAreTangentsComputed)
		: Name(std::move(InName))
		, Vertices(std::move(InVertices))
		, Indices(std::move(InIndices))
		, HasBeenTriangulated(CheckIfIsTriangulated())
		, AreTangentsComputed(InAreTangentsComputed)
	{
	}

	bool Mesh::HasTangents() const
	{
		return AreTangentsComputed;
	}

	bool Mesh::IsTriangulated() const
	{
		return HasBeenTriangulated;
	}

	void Mesh::FlipVertexOrder()
	{
		size_t IndexOfFistVertexInCurrentPolygon = 0;
		for (size_t Index = 0; Index < Indices.size(); Index++)
		{
			if (Indices[Index] == static_cast<uint32>(-1))
			{
				std::reverse(&Indices[IndexOfFistVertexInCurrentPolygon], &Indices[Index]);
				IndexOfFistVertexInCurrentPolygon = Index + 1;
			}
		}
	}

	void Mesh::ComputeTangents()
	{
		std::vector CountOfVertexOccurrences(Vertices.size(), 0.0f);

		// NOTE : adding 4 because each triangle is separated by -1 in the index array (e.g. I1, I2, I3, -1, I1, I2...)
		for (size_t VertexIndex = 0; VertexIndex < Indices.size(); VertexIndex += 4)
		{
			auto& V1 = Vertices[Indices[VertexIndex + 0]];
			auto& V2 = Vertices[Indices[VertexIndex + 1]];
			auto& V3 = Vertices[Indices[VertexIndex + 2]];

			auto T1 = CalculateSingleTangent(V1, V2, V3);
			auto T2 = CalculateSingleTangent(V2, V3, V1);
			auto T3 = CalculateSingleTangent(V3, V1, V2);

			V1.Tangent = { V1.Tangent.X + T1.X, V1.Tangent.Y + T1.Y, V1.Tangent.Z + T1.Z };
			V2.Tangent = { V2.Tangent.X + T2.X, V2.Tangent.Y + T2.Y, V2.Tangent.Z + T2.Z };
			V3.Tangent = { V3.Tangent.X + T3.X, V3.Tangent.Y + T3.Y, V3.Tangent.Z + T3.Z };

			CountOfVertexOccurrences[Indices[VertexIndex + 0]] += 1.0f;
			CountOfVertexOccurrences[Indices[VertexIndex + 1]] += 1.0f;
			CountOfVertexOccurrences[Indices[VertexIndex + 2]] += 1.0f;
		}

		for (size_t VertexIndex = 0; VertexIndex < Vertices.size(); VertexIndex++)
		{
			auto& Vertex = Vertices[VertexIndex];
			float Denominator = CountOfVertexOccurrences[VertexIndex];
			Vertex.Tangent = {
				Vertex.Tangent.X / Denominator, Vertex.Tangent.Y / Denominator, Vertex.Tangent.Z / Denominator
			};
		}

		AreTangentsComputed = true;
	}

	StringView Mesh::GetName() const
	{
		return Name;
	}

	const std::vector<Vertex>& Mesh::GetVertices() const
	{
		return Vertices;
	}

	const std::vector<uint32>& Mesh::GetIndices() const
	{
		return Indices;
	}

	bool Mesh::CheckIfIsTriangulated() const
	{
		// NOTE: this expects that the mesh is valid (e.g. it doesn't have faces with < 3 vertices)
		for (size_t Index = 0; Index < Indices.size(); Index++)
		{
			if (Index % 4 == 3 && Indices[Index] != static_cast<uint32>(-1))
				return false;
		}
		return true;
	}
}
