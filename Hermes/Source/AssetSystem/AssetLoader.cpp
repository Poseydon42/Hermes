#include "AssetLoader.h"

#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/AssetHeaders.h"
#include "JSON/JSONParser.h"
#include "Logging/Logger.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	std::unordered_map<AssetType, AssetLoader::BinaryAssetLoaderFunction> AssetLoader::BinaryLoaders;
	std::unordered_map<String, AssetLoader::TextAssetLoaderFunction> AssetLoader::TextLoaders;

	void AssetLoader::RegisterBinaryAssetLoader(AssetType Type, BinaryAssetLoaderFunction Loader)
	{
		// NOTE: this function will be called well before the engine is initialized, so we can't use the logger here
		HERMES_ASSERT(!BinaryLoaders.contains(Type));
		BinaryLoaders[Type] = Loader;
	}

	void AssetLoader::RegisterTextAssetLoader(String Type, TextAssetLoaderFunction Loader)
	{
		HERMES_ASSERT(!TextLoaders.contains(Type));
		TextLoaders[std::move(Type)] = Loader;
	}

	AssetHandle<Asset> AssetLoader::Load(StringView Name)
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
			return LoadBinary(*File, Name);
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

		return LoadText(*MaybeJSONRoot.value(), Name);
	}

	AssetHandle<Asset> AssetLoader::LoadBinary(IPlatformFile& File, StringView Name)
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
		return Loader(String(Name), AssetData);
	}

	AssetHandle<Asset> AssetLoader::LoadText(const JSONObject& JSONRoot, StringView Name)
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
		auto Type = String(Meta["type"].AsString());

		std::vector<StringView> DependencyNames;
		if (Meta.Contains("dependencies") && Meta["dependencies"].Is(JSONValueType::Array))
		{
			for (const auto& Dependency : Meta["dependencies"].AsArray())
			{
				if (!Dependency.Is(JSONValueType::String))
				{
					HERMES_LOG_ERROR("Dependency name is not a string (asset %s)", Name.data());
					return nullptr;
				}

				DependencyNames.push_back(Dependency.AsString());
			}
		}

		std::vector<AssetHandle<Asset>> DependencyHandles;
		auto& AssetCache = GGameLoop->GetAssetCache();
		for (const auto& DependencyName : DependencyNames)
		{
			auto DependencyHandle = AssetCache.Get<Asset>(String(DependencyName));
			if (!DependencyHandle)
			{
				HERMES_LOG_ERROR("Asset %s depends on asset %s which cannot be loaded", DependencyName.data());
				return nullptr;
			}
			DependencyHandles.push_back(DependencyHandle);
		}

		if (!JSONRoot.Contains("data") || !JSONRoot["data"].Is(JSONValueType::Object))
		{
			HERMES_LOG_ERROR("Asset %s does not provide any data", Name.data());
			return nullptr;
		}

		const auto& Data = JSONRoot["data"].AsObject();
		if (!TextLoaders.contains(Type))
		{
			HERMES_LOG_ERROR("Cannot load text asset of type \"%s\"", Type.c_str());
			return nullptr;
		}

		auto Loader = TextLoaders[Type];

		AssetLoaderCallbackInfo CallbackInfo = {
			.Name = Name,
			.Dependencies = std::move(DependencyHandles)
		};

		return Loader(CallbackInfo, Data);
	}
}
