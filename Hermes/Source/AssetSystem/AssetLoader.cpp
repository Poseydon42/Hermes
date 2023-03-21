#include "AssetLoader.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "AssetSystem/ImageAsset.h"
#include "AssetSystem/MaterialAsset.h"
#include "AssetSystem/MeshAsset.h"
#include "JSON/JSONParser.h"
#include "Logging/Logger.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	static AssetType AssetTypeFromString(StringView String)
	{
		if (String == "material")
			return AssetType::Material;
		return AssetType::Invalid;
	}
	
	std::unique_ptr<Asset> AssetLoader::Load(StringView Name)
	{
		String Filename = String(Name) + ".hac";

		auto File = VirtualFilesystem::Open(Filename, FileOpenMode::OpenExisting, FileAccessMode::Read);
		if (!File)
		{
			HERMES_LOG_WARNING("Failed to open asset file %s", Name.data());
			return nullptr;
		}

		uint8 Signature[sizeof(AssetHeader::ExpectedSignature)];
		if (File->Read(Signature, sizeof(Signature)) && memcmp(Signature, AssetHeader::ExpectedSignature, sizeof(Signature)) == 0)
		{
			File->Seek(0);
			return LoadBinary(*File, Name);
		}

		String FileContents(File->Size(), 0);
		File->Seek(0);
		if (!File->Read(FileContents.data(), FileContents.size()))
		{
			HERMES_LOG_WARNING("Cannot read asset file %s", Name.data());
			return nullptr;
		}

		auto MaybeJSONRoot = JSONParser::FromString(FileContents);
		if (!MaybeJSONRoot.has_value() || !MaybeJSONRoot.value())
		{
			HERMES_LOG_WARNING("Asset %s is assumed to be text, but it cannot be parsed as JSON", Name.data());
			return nullptr;
		}

		return LoadText(*MaybeJSONRoot.value(), Name);
	}

	std::unique_ptr<Asset> AssetLoader::LoadBinary(IPlatformFile& File, StringView Name)
	{
		AssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read asset header from asset file %s", Name.data());
			return nullptr;
		}

		if (memcmp(Header.Signature, AssetHeader::ExpectedSignature, sizeof(Header.Signature)) != 0)
		{
			HERMES_LOG_WARNING("Asset %s has invalid header signature", Name.data());
			return nullptr;
		}

		switch (Header.Type)
		{
		case AssetType::Image:
			return LoadImage(File, Header, Name);
		case AssetType::Mesh:
			return LoadMesh(File, Header, Name);
		default:
			HERMES_LOG_WARNING("Binary asset %s (type %02x) cannot be loaded", Name.data(), static_cast<uint8>(Header.Type));
			return nullptr;
		}
	}

	std::unique_ptr<Asset> AssetLoader::LoadText(const JSONObject& JSONRoot, StringView Name)
	{
		if (!JSONRoot.Contains("meta") || !JSONRoot["meta"].Is(JSONValueType::Object))
		{
			HERMES_LOG_WARNING("Asset %s does not contains 'meta' JSON property", Name.data());
			return nullptr;
		}

		const auto& Meta = JSONRoot["meta"].AsObject();

		if (!Meta.Contains("type") || !Meta["type"].Is(JSONValueType::String))
		{
			HERMES_LOG_WARNING("Asset's %s meta does not specify its type", Name.data());
			return nullptr;
		}

		auto StringType = Meta["type"].AsString();
		auto Type = AssetTypeFromString(StringType);

		if (!JSONRoot.Contains("data") || !JSONRoot["data"].Is(JSONValueType::Object))
		{
			HERMES_LOG_WARNING("Asset %s does not provide any data", Name.data());
			return nullptr;
		}

		const auto& Data = JSONRoot["data"].AsObject();
		switch (Type)
		{
		case AssetType::Material:
			return LoadMaterial(Data, Name);
		default:
			HERMES_LOG_WARNING("Cannot load text asset of type %02x", static_cast<uint8>(Type));
			return nullptr;
		}
	}

	std::unique_ptr<Asset> AssetLoader::LoadMaterial(const JSONObject& Data, StringView Name)
	{
		if (!Data.Contains("shaders") || !Data["shaders"].Is(JSONValueType::Object))
		{
			HERMES_LOG_WARNING("Material asset %s does not specify any shaders", Name.data());
			return nullptr;
		}

		const auto& JSONShaders = Data["shaders"].AsObject();

		std::vector<std::pair<String, String>> Shaders;
		for (const auto& JSONShader : JSONShaders)
		{
			if (!JSONShader.second.Is(JSONValueType::String))
				continue;

			const auto& Type = JSONShader.first;
			auto Path = JSONShader.second.AsString();

			Shaders.emplace_back(Type, String(Path));
		}
		
		return std::unique_ptr<MaterialAsset>(new MaterialAsset(String(Name), Shaders));
	}

	std::unique_ptr<Asset> AssetLoader::LoadImage(IPlatformFile& File, const AssetHeader&, StringView Name)
	{
		ImageAssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read image header from asset %s", Name.data());
			return nullptr;
		}

		size_t BytesPerPixel = NumberOfChannelInImageFormat(Header.Format) * Header.BytesPerChannel;
		size_t TotalBytes = CalculateTotalPixelCount(Header.Width, Header.Height, Header.MipLevelCount) * BytesPerPixel;
		std::vector<uint8> ImageData(TotalBytes, 0x00);
		if (!File.Read(ImageData.data(), TotalBytes))
		{
			HERMES_LOG_WARNING("Failed to read image data from asset %s", Name.data());
			return nullptr;
		}

		auto Result = std::unique_ptr<ImageAsset>(new ImageAsset(String(Name), Vec2ui{Header.Width, Header.Height},
		                                                         Header.Format, Header.BytesPerChannel, Header.MipLevelCount,
		                                                         ImageData.data()));

		return Result;
	}

	std::unique_ptr<Asset> AssetLoader::LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, StringView Name)
	{
		(void)AssetHeader;

		MeshAssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read mesh header from asset %s", Name.data());
			return nullptr;
		}

		std::vector<MeshPrimitiveHeader> PrimitiveHeaders(Header.PrimitiveCount);
		if (!File.Read(PrimitiveHeaders.data(), PrimitiveHeaders.size() * sizeof(PrimitiveHeaders[0])))
		{
			HERMES_LOG_WARNING("Could not read mesh primitive list from mesh %s", Name.data());
			return nullptr;
		}

		std::vector<MeshAsset::Primitive> Primitives(PrimitiveHeaders.size());
		for (size_t Index = 0; Index < Primitives.size(); Index++)
		{
			Primitives[Index].IndexBufferOffset = PrimitiveHeaders[Index].IndexBufferOffset;
			Primitives[Index].IndexCount = PrimitiveHeaders[Index].IndexCount;
		}

		std::vector<Vertex> Vertices(Header.VertexBufferSize);
		if (!File.Read(Vertices.data(), Vertices.size() * sizeof(Vertex)))
		{
			HERMES_LOG_WARNING("Could not read vertex buffer from mesh %s", Name.data());
			return nullptr;
		}

		std::vector<uint32> Indices(Header.IndexBufferSize);
		if (!File.Read(Indices.data(), Indices.size() * sizeof(Indices[0])))
		{
			HERMES_LOG_WARNING("Could not read index buffer from mesh %s", Name.data());
			return nullptr;
		}

		return std::unique_ptr<MeshAsset>(new MeshAsset(String(Name), Vertices, Indices, Primitives));
	}
}
