#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>

/*
 * TODO : setup our build so that we can use our own platform layer
 * for file IO etc. instead of using STL
 * Currently we can't do that because our engine builds as one gigantic module
 * and so we have to run full engine startup in order to use platform layer properly,
 * which we don't want obviously.
 */

#ifdef HERMES_PLATFORM_WINDOWS
#define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
#define PACKED_STRUCT_END   __pragma(pack(pop))
#else
#error "Unsupported platform"
#endif

// NOTE : keep in sync with Hermes' source code until we'll be able to include its headers inside tools sources
enum class AssetType : uint8_t
{
	Image = 1,
	Mesh = 2
};

PACKED_STRUCT_BEGIN
struct AssetHeader
{
	AssetType Type;
};

struct MeshHeader
{
	uint32_t VertexCount;
	uint32_t IndexCount;
};
PACKED_STRUCT_END

struct Vec3
{
	float X, Y, Z;
};

struct Vec2
{
	float X, Y;
};

struct Vertex
{
	Vec3 Position;
	Vec2 TextureCoordinates;
	Vec3 Normal;
};

int WriteAssetFile(const std::string& FileName, const std::vector<Vertex>& Vertices, const std::vector<uint32_t>& Indices)
{
	AssetHeader AssetHeader;
	AssetHeader.Type = AssetType::Mesh;

	MeshHeader Header;
	Header.VertexCount = static_cast<uint32_t>(Vertices.size());
	Header.IndexCount = static_cast<uint32_t>(Indices.size());

	std::ofstream Output(FileName, std::ios::out | std::ios::binary);
	if (!Output.is_open())
	{
		std::cerr << "Failed to open file " << FileName << " for writing" << std::endl;
		return 4;
	}

	Output.write(reinterpret_cast<const char*>(&AssetHeader), sizeof(AssetHeader));
	Output.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	Output.write(reinterpret_cast<const char*>(Vertices.data()), static_cast<uint32_t>(Vertices.size() * sizeof(Vertices[0])));
	Output.write(reinterpret_cast<const char*>(Indices.data()), static_cast<uint32_t>(Indices.size() * sizeof(Indices[0])));

	return 0;
}

int ConvertFromOBJ(const std::string& InputFile, const std::string& OutputFilename, bool FlipTriangleOrder)
{
	std::ifstream Input = std::ifstream(InputFile, std::ios::in | std::ios::binary);
	if (!Input.is_open())
	{
		std::cerr << "Failed to open file " << InputFile << " for reading" << std::endl;
		return 3;
	}

	std::vector<Vec3> VertexPositions;
	std::vector<Vec2> VertexTextureCoordinates;
	std::vector<Vec3> VertexNormals;
	struct UniqueVertex
	{
		size_t PositionIndex;
		size_t TextureCoordinatesIndex;
		size_t NormalIndex;

		bool operator==(const UniqueVertex& Other) const { return !memcmp(this, &Other, sizeof(*this)); }
	};

	std::vector<UniqueVertex> UniqueVertices;
	std::vector<uint32_t> ResultIndices;
	std::vector<Vertex> ResultVertices;
	
	std::string CurrentLine;
	uint32_t LineNumber = 0;
	while (std::getline(Input, CurrentLine))
	{
		LineNumber++;
		if (CurrentLine.empty())
			continue;
		std::istringstream LineStream(CurrentLine);
		std::string ValueType;
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
			char DummySlashChar;
			UniqueVertex V1, V2, V3;
			LineStream >>
				V1.PositionIndex >> DummySlashChar >> V1.TextureCoordinatesIndex >> DummySlashChar >> V1.NormalIndex >>
				V2.PositionIndex >> DummySlashChar >> V2.TextureCoordinatesIndex >> DummySlashChar >> V2.NormalIndex >>
				V3.PositionIndex >> DummySlashChar >> V3.TextureCoordinatesIndex >> DummySlashChar >> V3.NormalIndex;

			// Because in OBJ file indexing starts from 1
			V1.PositionIndex--;
			V1.TextureCoordinatesIndex--;
			V1.NormalIndex--;
			V2.PositionIndex--;
			V2.TextureCoordinatesIndex--;
			V2.NormalIndex--;
			V3.PositionIndex--;
			V3.TextureCoordinatesIndex--;
			V3.NormalIndex--;

			if (FlipTriangleOrder)
				std::swap(V1, V3);

			auto InsertVertexAndFindIndex = [&UniqueVertices](const UniqueVertex& CurrentVertex) -> uint32_t
			{
				auto Iterator = std::find(UniqueVertices.begin(), UniqueVertices.end(), CurrentVertex);
				if (Iterator == UniqueVertices.end())
				{
					UniqueVertices.push_back(CurrentVertex);
					return static_cast<uint32_t>(std::distance(UniqueVertices.begin(), UniqueVertices.end() - 1));
				}
				return static_cast<uint32_t>(std::distance(UniqueVertices.begin(), Iterator));
			};
			
			ResultIndices.push_back(InsertVertexAndFindIndex(V1));
			ResultIndices.push_back(InsertVertexAndFindIndex(V2));
			ResultIndices.push_back(InsertVertexAndFindIndex(V3));
		}
		else if (ValueType == "s")
		{
			std::cout << "Smoothing groups information is currently ignored(line " << LineNumber << ")" << std::endl;
		}
		else
		{
			std::cerr << "Unknown value type specifier " << ValueType << " at line " << LineNumber << ", this line will be skipped" << std::endl;
		}
	}
	
	for (const auto& CurrentVertex : UniqueVertices)
	{
		Vertex NewVertex;
		NewVertex.Position = VertexPositions[CurrentVertex.PositionIndex];
		NewVertex.TextureCoordinates = VertexTextureCoordinates[CurrentVertex.TextureCoordinatesIndex];
		NewVertex.Normal = VertexNormals[CurrentVertex.NormalIndex];
		ResultVertices.push_back(NewVertex);
	}
	
	return WriteAssetFile(OutputFilename, ResultVertices, ResultIndices);
}

/*
 * Usage:
 * meshtoasset <input file> [options]
 * Possible options:
 *  --obj: override file extension and parse it as OBJ file
 *	--flip, -f: flip triangle order(clockwise vs counterclockwise)
 * Currently supported mesh formats:
 *  - OBJ
 */
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "You need to specify input file" << std::endl;
		return 1;
	}

	enum class FileType
	{
		Undefined,
		OBJ
	} FileType = FileType::Undefined;

	bool FlipTriangleOrder = false;
	std::string InputFilename = std::string(argv[1]);
	std::string InputFileNameWithoutExtension = InputFilename.substr(0, InputFilename.find_last_of('.'));
	for (int ArgumentIndex = 2; ArgumentIndex < argc; ArgumentIndex++)
	{
		if (strcmp(argv[ArgumentIndex], "--obj") == 0)
			FileType = FileType::OBJ;
		if (strcmp(argv[ArgumentIndex], "--flip") == 0 ||
			strcmp(argv[ArgumentIndex], "-f") == 0)
			FlipTriangleOrder = true;
	}
	if (InputFilename.substr(InputFilename.find_last_of('.'), InputFilename.length() - InputFilename.find_last_of('.') + 1) == ".obj")
	{
		FileType = FileType::OBJ;
	}

	std::string OutputFileName = InputFileNameWithoutExtension + ".hac";

	switch (FileType)
	{
	case FileType::OBJ:
		return ConvertFromOBJ(InputFilename, OutputFileName, FlipTriangleOrder);
	case FileType::Undefined:
	default:
		std::cerr << "Unknown file format" << std::endl;
		return 2;
	}
}
