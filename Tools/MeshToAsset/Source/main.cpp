#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <cmath>

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

// TODO : find a way to use math from engine and not reinvent the wheel here
Vec3 operator-(const Vec3& Left, const Vec3& Right)
{
	return { Left.X - Right.X, Left.Y - Right.Y, Left.Z - Right.Z };
}

Vec3 operator*(float Coefficient, const Vec3& Vec)
{
	return { Coefficient * Vec.X, Coefficient * Vec.Y, Coefficient * Vec.Z };
}

Vec3 operator/(const Vec3& Numerator, float Denominator)
{
	return (1.0f / Denominator) * Numerator;
}

Vec3 Normalize(const Vec3& Input)
{
	float Length = std::sqrt(Input.X * Input.X + Input.Y * Input.Y + Input.Z * Input.Z);

	return { Input.X / Length, Input.Y / Length, Input.Z / Length };
}

struct Vec2
{
	float X, Y;
};

struct Vertex
{
	Vec3 Position;
	Vec2 TextureCoordinates;
	Vec3 Normal;
	Vec3 Tangent;
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

/*
 * Calculates tangent vector of a single vertex V1 that is inside a triangle formed
 * by vertices V1, V2 and V3 in counterclockwise order
 * NOTE : this calculation does not care about smooth edges and shared vertices - it's caller
 *        responsibility to average tangents of a shared vertex
 */
Vec3 CalculateSingleTangent(const Vertex& V1, const Vertex& V2, const Vertex& V3)
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

	Vec3 Numerator = DeltaUV12.Y * Edge13 - DeltaUV13.Y * Edge12;
	float Denominator = DeltaUV12.Y * DeltaUV13.X - DeltaUV12.X * DeltaUV13.Y;
	// Use default UV direction when denominator is 0
	if (Denominator <= 0.000001f)
	{
		DeltaUV12 = { 0.0f, 1.0f };
		DeltaUV13 = { 1.0f, 0.0f };
		Denominator = 1.0f;
	}

	Vec3 Result = Normalize(Numerator / Denominator);

	return Result;
}

void CalculateTangents(std::vector<Vertex>& Vertices, const std::vector<uint32_t>& Indices)
{
	std::vector CountOfVertexOccurrences(Vertices.size(), 0.0f);

	// NOTE : looping over all triangles
	for (size_t VertexIndex = 0; VertexIndex < Indices.size(); VertexIndex += 3)
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
		Vertex NewVertex = {};
		NewVertex.Position = VertexPositions[CurrentVertex.PositionIndex];
		NewVertex.TextureCoordinates = VertexTextureCoordinates[CurrentVertex.TextureCoordinatesIndex];
		NewVertex.Normal = VertexNormals[CurrentVertex.NormalIndex];
		ResultVertices.push_back(NewVertex);
	}

	CalculateTangents(ResultVertices, ResultIndices);
	
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
