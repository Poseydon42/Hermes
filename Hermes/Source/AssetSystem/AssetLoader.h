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
		static std::unique_ptr<Asset> Load(StringView Name, AssetHandle Handle);

	private:
		static std::unique_ptr<Asset> LoadBinary(IPlatformFile& File, StringView Name, AssetHandle Handle);

		static std::unique_ptr<Asset> LoadImage(IPlatformFile& File, const AssetHeader& Header, StringView Name, AssetHandle Handle);

		static std::unique_ptr<Asset> LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, StringView Name, AssetHandle Handle);


		static std::unique_ptr<Asset> LoadText(const JSONObject& JSONRoot, StringView Name, AssetHandle Handle);

		static std::unique_ptr<Asset> LoadMaterial(const JSONObject& Data, StringView Name, AssetHandle Handle);
	};
}
