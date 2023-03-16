#include "OBJReader.h"

#include <algorithm>
#include <iostream>
#include <numeric>

#include "AssetSystem/MeshAsset.h"
#include "Mesh.h"
#include "Node.h"
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

		String CurrentMeshName;
		std::vector<UniqueVertex> CurrentMeshVertices;
		std::vector<uint32> CurrentMeshIndices;

		auto AddMeshAndResetBuffers = [&]()
		{
			std::vector<Vertex> ComputedVertices;
			for (const auto& CurrentVertex : CurrentMeshVertices)
			{
				Vertex NewVertex = {};
				NewVertex.Position = VertexPositions[CurrentVertex.PositionIndex];
				NewVertex.TextureCoordinates = VertexTextureCoordinates[CurrentVertex.TextureCoordinatesIndex];
				NewVertex.Normal = VertexNormals[CurrentVertex.NormalIndex];
				ComputedVertices.push_back(NewVertex);
			}
			HERMES_ASSERT(ComputedVertices.size() < std::numeric_limits<uint32>::max());

			MeshPrimitiveHeader Primitive = { .IndexBufferOffset = 0, .IndexCount = static_cast<uint32>(CurrentMeshIndices.size()) };

			Meshes.emplace_back(CurrentMeshName, std::move(ComputedVertices), std::move(CurrentMeshIndices), std::vector{ Primitive }, false);

			Root->AddChild(Node::Create(CurrentMeshName, Mat4::Identity(), CurrentMeshName, NodePayloadType::Mesh));
			
			CurrentMeshVertices.clear();
			CurrentMeshIndices.clear();
		};

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

			if (ValueType == "o")
			{
				String NewMeshName;
				LineStream >> NewMeshName;
				if (CurrentMeshName.empty()) // NOTE: meaning that this is the first mesh declaration in the file
				{
					CurrentMeshName = NewMeshName;
					continue;
				}
				else
				{
					AddMeshAndResetBuffers();
					CurrentMeshName = NewMeshName;
				}
			}
			else if (ValueType == "v")
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

				auto InsertVertexAndIndex = [&](const UniqueVertex& CurrentVertex)
				{
					auto Iterator = std::ranges::find(CurrentMeshVertices, CurrentVertex);
					if (Iterator == CurrentMeshVertices.end())
					{
						CurrentMeshVertices.push_back(CurrentVertex);
						Iterator = CurrentMeshVertices.end() - 1;
					}
					CurrentMeshIndices.push_back(static_cast<uint32>(std::distance(CurrentMeshVertices.begin(), Iterator)));
				};

				std::ranges::for_each(FaceVertices, InsertVertexAndIndex);
				CurrentMeshIndices.push_back(static_cast<uint32>(-1)); // Face separator
			}

		}

		AddMeshAndResetBuffers();

		return true;
	}

	const Node& OBJReader::GetRootNode() const
	{
		return *Root;
	}

	std::optional<const Mesh*> OBJReader::GetMesh(StringView MeshName) const
	{
		for (const auto& Mesh : Meshes)
		{
			if (Mesh.GetName() == MeshName)
				return &Mesh;
		}
		return {};
	}
}
