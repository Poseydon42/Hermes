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

	class HERMES_API AssetLoader
	{
	public:
		using BinaryAssetLoaderFunction = std::unique_ptr<Asset>(*)(String, AssetHandle, std::span<const uint8>);
		using TextAssetLoaderFunction = std::unique_ptr<Asset>(*)(String, AssetHandle, const JSONObject&);

		static void RegisterBinaryAssetLoader(AssetType Type, BinaryAssetLoaderFunction Loader);
		static void RegisterTextAssetLoader(String Type, TextAssetLoaderFunction Loader);

		static std::unique_ptr<Asset> Load(StringView Name, AssetHandle Handle);

	private:
		static std::unordered_map<AssetType, BinaryAssetLoaderFunction> BinaryLoaders;
		static std::unordered_map<String, TextAssetLoaderFunction> TextLoaders;

		static std::unique_ptr<Asset> LoadBinary(IPlatformFile& File, StringView Name, AssetHandle Handle);

		static std::unique_ptr<Asset> LoadText(const JSONObject& JSONRoot, StringView Name, AssetHandle Handle);
	};
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
