#pragma once

#include <memory>

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetHeaders.h"
#include "Core/Core.h"
#include "JSON/JSONObject.h"

namespace Hermes
{
	class IPlatformFile;

	class HERMES_API AssetLoader
	{
	public:
		using BinaryAssetLoaderFunction = std::unique_ptr<Asset>(*)(String, AssetHandle, std::span<const uint8>);

		static void RegisterBinaryAssetLoader(AssetType Type, BinaryAssetLoaderFunction Loader);

		static std::unique_ptr<Asset> Load(StringView Name, AssetHandle Handle);

	private:
		static std::unordered_map<AssetType, BinaryAssetLoaderFunction> BinaryLoaders;

		static std::unique_ptr<Asset> LoadBinary(IPlatformFile& File, StringView Name, AssetHandle Handle);


		static std::unique_ptr<Asset> LoadText(const JSONObject& JSONRoot, StringView Name, AssetHandle Handle);

		static std::unique_ptr<Asset> LoadMaterial(const JSONObject& Data, StringView Name, AssetHandle Handle);
	};
}
