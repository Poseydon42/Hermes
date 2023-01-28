#pragma once

#include <memory>

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetHeaders.h"
#include "AssetSystem/ImageAsset.h"
#include "Core/Core.h"

namespace Hermes
{
	class IPlatformFile;

	class HERMES_API AssetLoader
	{
	public:
		static std::unique_ptr<Asset> Load(StringView Name);

	private:

		static std::unique_ptr<Asset> LoadImage(IPlatformFile& File, const AssetHeader& Header, StringView Name);

		static std::unique_ptr<Asset> LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, StringView Name);
	};
}
