#pragma once

#include <memory>

#include "AssetSystem/Asset.h"
#include "AssetSystem/ImageAsset.h"
#include "Core/Core.h"

namespace Hermes
{
	class IPlatformFile;

	PACKED_STRUCT_BEGIN
	struct AssetHeader
	{
		AssetType Type;
		// TODO : version, dependencies etc.
	};
	PACKED_STRUCT_END

	class HERMES_API AssetLoader
	{
	public:
		static std::shared_ptr<Asset> Load(StringView Name);

	private:

		static std::shared_ptr<Asset> LoadImage(IPlatformFile& File, const AssetHeader& Header, StringView Name);

		static std::shared_ptr<Asset> LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, StringView Name);
	};
}
