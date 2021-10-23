#pragma once

#include <memory>

#include "AssetSystem/Asset.h"
#include "AssetSystem/ImageAsset.h"
#include "Core/Core.h"

namespace Hermes
{
	class IPlatformFile;

	class HERMES_API AssetLoader
	{
	public:
		static std::shared_ptr<Asset> Load(const String& Name);

	private:
		PACKED_STRUCT_BEGIN
		struct AssetHeader
		{
			AssetType Type;
			// TODO : version, dependencies etc.
		};
		PACKED_STRUCT_END

		static std::shared_ptr<Asset> LoadImage(IPlatformFile& File, const AssetHeader& Header, const String& Name);
	};
}
