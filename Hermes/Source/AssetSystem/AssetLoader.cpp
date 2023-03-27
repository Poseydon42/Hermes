﻿#include "AssetLoader.h"

#include "JSON/JSONParser.h"
#include "Logging/Logger.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "RenderingEngine/Material/Material.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	static AssetType AssetTypeFromString(StringView String)
	{
		if (String == "material")
			return AssetType::Material;
		return AssetType::Invalid;
	}

	std::unordered_map<AssetType, AssetLoader::BinaryAssetLoaderFunction> AssetLoader::BinaryLoaders;

	void AssetLoader::RegisterBinaryAssetLoader(AssetType Type, BinaryAssetLoaderFunction Loader)
	{
		// NOTE: this function will be called well before the engine is initialized, so we can't use the logger here
		HERMES_ASSERT(!BinaryLoaders.contains(Type));
		BinaryLoaders[Type] = Loader;
	}

	std::unique_ptr<Asset> AssetLoader::Load(StringView Name, AssetHandle Handle)
	{
		String Filename = String(Name) + ".hac";

		auto File = VirtualFilesystem::Open(Filename, FileOpenMode::OpenExisting, FileAccessMode::Read);
		if (!File)
		{
			HERMES_LOG_ERROR("Failed to open asset file %s", Name.data());
			return nullptr;
		}

		uint8 Signature[sizeof(AssetHeader::ExpectedSignature)];
		if (File->Read(Signature, sizeof(Signature)) && memcmp(Signature, AssetHeader::ExpectedSignature, sizeof(Signature)) == 0)
		{
			File->Seek(0);
			return LoadBinary(*File, Name, Handle);
		}

		String FileContents(File->Size(), 0);
		File->Seek(0);
		if (!File->Read(FileContents.data(), FileContents.size()))
		{
			HERMES_LOG_ERROR("Cannot read asset file %s", Name.data());
			return nullptr;
		}

		auto MaybeJSONRoot = JSONParser::FromString(FileContents);
		if (!MaybeJSONRoot.has_value() || !MaybeJSONRoot.value())
		{
			HERMES_LOG_ERROR("Asset %s is assumed to be text, but it cannot be parsed as JSON", Name.data());
			return nullptr;
		}

		return LoadText(*MaybeJSONRoot.value(), Name, Handle);
	}

	std::unique_ptr<Asset> AssetLoader::LoadBinary(IPlatformFile& File, StringView Name, AssetHandle Handle)
	{
		AssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_ERROR("Failed to read asset header from asset file %s", Name.data());
			return nullptr;
		}

		if (memcmp(Header.Signature, AssetHeader::ExpectedSignature, sizeof(Header.Signature)) != 0)
		{
			HERMES_LOG_ERROR("Asset %s has invalid header signature", Name.data());
			return nullptr;
		}

		if (!BinaryLoaders.contains(Header.Type))
		{
			HERMES_LOG_ERROR("Binary asset %s (type %02x) cannot be loaded: no corresponding loader found", Name.data(), static_cast<uint8>(Header.Type));
			return nullptr;
		}

		std::vector<uint8> AssetData(File.Size() - File.Tell());
		if (!File.Read(AssetData.data(), AssetData.size()))
		{
			HERMES_LOG_ERROR("Cannot read contenst of binary asset %s: file read failed", Name.data());
			return nullptr;
		}

		auto Loader = BinaryLoaders[Header.Type];
		return Loader(String(Name), Handle, AssetData);
	}

	std::unique_ptr<Asset> AssetLoader::LoadText(const JSONObject& JSONRoot, StringView Name, AssetHandle Handle)
	{
		if (!JSONRoot.Contains("meta") || !JSONRoot["meta"].Is(JSONValueType::Object))
		{
			HERMES_LOG_ERROR("Asset %s does not contains 'meta' JSON property", Name.data());
			return nullptr;
		}

		const auto& Meta = JSONRoot["meta"].AsObject();

		if (!Meta.Contains("type") || !Meta["type"].Is(JSONValueType::String))
		{
			HERMES_LOG_ERROR("Asset's %s meta does not specify its type", Name.data());
			return nullptr;
		}

		auto StringType = Meta["type"].AsString();
		auto Type = AssetTypeFromString(StringType);

		if (!JSONRoot.Contains("data") || !JSONRoot["data"].Is(JSONValueType::Object))
		{
			HERMES_LOG_ERROR("Asset %s does not provide any data", Name.data());
			return nullptr;
		}

		const auto& Data = JSONRoot["data"].AsObject();
		switch (Type)
		{
		case AssetType::Material:
			return LoadMaterial(Data, Name, Handle);
		default:
			HERMES_LOG_ERROR("Cannot load text asset of type %02x", static_cast<uint8>(Type));
			return nullptr;
		}
	}

	std::unique_ptr<Asset> AssetLoader::LoadMaterial(const JSONObject& Data, StringView Name, AssetHandle Handle)
	{
		if (!Data.Contains("shaders") || !Data["shaders"].Is(JSONValueType::Object))
		{
			HERMES_LOG_ERROR("Material asset %s does not specify any shaders", Name.data());
			return nullptr;
		}

		const auto& JSONShaders = Data["shaders"].AsObject();

		String VertexShader, FragmentShader;
		for (const auto& JSONShader : JSONShaders)
		{
			if (!JSONShader.second.Is(JSONValueType::String))
				continue;

			const auto& Type = JSONShader.first;
			auto Path = JSONShader.second.AsString();

			if (Type == "vertex")
				VertexShader = String(Path);
			if (Type == "fragment")
				FragmentShader = String(Path);
		}
		
		return std::unique_ptr<Material>(new Material(String(Name), Handle, VertexShader, FragmentShader));
	}
}
