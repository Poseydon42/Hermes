#include "OBJReader.h"

#include <algorithm>
#include <iostream>

#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Tools
{
	struct UniqueVertex
	{
		size_t PositionIndex;
		size_t TextureCoordinatesIndex;
		size_t NormalIndex;

		bool operator==(const UniqueVertex& Other) const
		{
			return PositionIndex == Other.PositionIndex && TextureCoordinatesIndex == Other.TextureCoordinatesIndex && NormalIndex == Other.NormalIndex;
		}
	};

	bool OBJReader::Read(StringView Path)
	{
		auto MaybeInputData = PlatformFilesystem::ReadFileAsString(Path);
		if (!MaybeInputData.has_value())
		{
			std::cerr << "Cannot read input file " << Path << std::endl;
			return false;
		}
		StringStream InputStream(MaybeInputData.value());

		std::vector<Vec3> VertexPositions;
		std::vector<Vec2> VertexTextureCoordinates;
		std::vector<Vec3> VertexNormals;
		std::vector<UniqueVertex> UniqueVertices;

		String CurrentLine;
		// FIXME: validate the contents of the file instead of just returning garbage
		while (std::getline(InputStream, CurrentLine))
		{
			if (CurrentLine.empty())
				continue;

			StringStream LineStream(CurrentLine);

			String ValueType;
			LineStream >> ValueType;

			if (ValueType.empty() || ValueType.find_first_of('#') == 0)
				continue;

			if (ValueType == "v")
			{
				Vec3 NewPosition;
				LineStream >> NewPosition.X >> NewPosition.Y >> NewPosition.Z;
				VertexPositions.push_back(NewPosition);
			}
			else if (ValueType == "vt")
			{
				Vec2 NewTextureCoordinates;
				LineStream >> NewTextureCoordinates.X >> NewTextureCoordinates.Y;
				VertexTextureCoordinates.push_back(NewTextureCoordinates);
			}
			else if (ValueType == "vn")
			{
				Vec3 NewNormal;
				LineStream >> NewNormal.X >> NewNormal.Y >> NewNormal.Z;
				VertexNormals.push_back(NewNormal);
			}
			else if (ValueType == "f")
			{
				std::vector<UniqueVertex> FaceVertices;
				while (!LineStream.eof())
				{
					char DummySlash;
					uint32 Position, TextureCoord, Normal;

					LineStream >> Position >> DummySlash >> TextureCoord >> DummySlash >> Normal;
					FaceVertices.emplace_back(Position - 1, TextureCoord - 1, Normal - 1); // NOTE: indexing in OBJ starts from 1
				}

				if (FaceVertices.size() < 3)
				{
					std::cerr << "Cannot parse OBJ file: met face with less than 3 vertices" << std::endl;
					return false;
				}
				if (FaceVertices.size() > 3)
				{
					HasNonTriangleFaces = true;
				}

				auto InsertVertexAndIndex = [&](const UniqueVertex& CurrentVertex)
				{
					auto Iterator = std::ranges::find(UniqueVertices, CurrentVertex);
					if (Iterator == UniqueVertices.end())
					{
						UniqueVertices.push_back(CurrentVertex);
						Iterator = UniqueVertices.end() - 1;
					}
					Indices.push_back(static_cast<uint32>(std::distance(UniqueVertices.begin(), Iterator)));
				};

				std::ranges::for_each(FaceVertices, InsertVertexAndIndex);
				Indices.push_back(static_cast<uint32>(-1)); // Face separator
			}
		}

		for (const auto& CurrentVertex : UniqueVertices)
		{
			Vertex NewVertex = {};
			NewVertex.Position = VertexPositions[CurrentVertex.PositionIndex];
			NewVertex.TextureCoordinates = VertexTextureCoordinates[CurrentVertex.TextureCoordinatesIndex];
			NewVertex.Normal = VertexNormals[CurrentVertex.NormalIndex];
			Vertices.push_back(NewVertex);
		}

		return true;
	}

	bool OBJReader::HasTangents() const
	{
		return false;
	}

	bool OBJReader::IsTriangulated() const
	{
		return !HasNonTriangleFaces;
	}

	std::span<const Vertex> OBJReader::GetVertices() const
	{
		return Vertices;
	}

	std::span<const uint32> OBJReader::GetIndices() const
	{
		return Indices;
	}
}
