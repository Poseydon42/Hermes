#pragma once

#include <memory>
#include <span>
#include <unordered_map>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"

namespace Hermes
{
	class JSONObject;
	class IPlatformFile;

	struct AssetLoaderCallbackInfo
	{
		StringView Name;
		std::vector<AssetHandle<Asset>> Dependencies;
	};

	class HERMES_API AssetLoader
	{
	public:
		using BinaryAssetLoaderFunction = AssetHandle<Asset>(*)(String, std::span<const uint8>);
		using TextAssetLoaderFunction = AssetHandle<Asset>(*)(const AssetLoaderCallbackInfo&, const JSONObject&);

		static void RegisterBinaryAssetLoader(AssetType Type, BinaryAssetLoaderFunction Loader);
		static void RegisterTextAssetLoader(String Type, TextAssetLoaderFunction Loader);

		template<typename AssetClass>
		requires (std::derived_from<AssetClass, Asset>)
		static AssetHandle<AssetClass> Load(const String& AssetName);

	private:
		static std::unordered_map<String, AssetHandle<Asset>> LoadedAssets;

		static std::unordered_map<AssetType, BinaryAssetLoaderFunction> BinaryLoaders;
		static std::unordered_map<String, TextAssetLoaderFunction> TextLoaders;

		static AssetHandle<Asset> LoadBinary(IPlatformFile& File, StringView Name);

		static AssetHandle<Asset> LoadText(const JSONObject& JSONRoot, StringView Name);

		static AssetHandle<Asset> LoadImpl(const String& Name);
	};

	template<typename AssetClass>
	requires (std::derived_from<AssetClass, Asset>)
	AssetHandle<AssetClass> AssetLoader::Load(const String& AssetName)
	{
		auto Result = LoadImpl(AssetName);

		if (Result->GetType() != AssetClass::GetStaticType())
			return nullptr;

		return AssetCast<AssetClass>(Result);
	}
}

#define HERMES_ADD_BINARY_ASSET_LOADER(ClassName, Type)                               \
	namespace Asset##Type##ClassName##BinaryLoaderInitializerInternal                 \
	{                                                                                 \
		static bool InitializeLoader()                                                \
		{                                                                             \
			AssetLoader::RegisterBinaryAssetLoader(AssetType::Type, ClassName::Load); \
			return true;                                                              \
		}                                                                             \
		static volatile [[maybe_unused]] bool InitializerDummy = InitializeLoader();  \
	}

#define HERMES_ADD_TEXT_ASSET_LOADER(ClassName, Type)                                \
	namespace Asset##ClassName##TextLoaderInitializerInternal                        \
	{                                                                                \
		static bool InitializeLoader()                                               \
		{                                                                            \
			AssetLoader::RegisterTextAssetLoader(Type, ClassName::Load);             \
			return true;                                                             \
		}                                                                            \
		static volatile [[maybe_unused]] bool InitializerDummy = InitializeLoader(); \
	}
