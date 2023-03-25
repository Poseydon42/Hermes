#include "AssetCache.h"

#include "AssetSystem/AssetLoader.h"
#include "Core/Profiling.h"

namespace Hermes
{
	AssetHandle AssetCache::Create(StringView AssetName)
	{
		HERMES_PROFILE_FUNC();
		for (const auto& [Handle, Asset] : LoadedAssets)
		{
			if (Asset->GetName() == AssetName)
				return Handle;
		}

		auto Result = LoadAsset(AssetName);
		return Result;
	}

	std::optional<const Asset*> AssetCache::GetImpl(AssetHandle Handle)
	{
		HERMES_PROFILE_FUNC();
		if (!LoadedAssets.contains(Handle))
			return nullptr;

		auto Asset = LoadedAssets.at(Handle).get();;
		if (!Asset)
			return nullptr;
		
		return Asset;
	}

	std::optional<const Asset*> AssetCache::GetImpl(StringView Name)
	{
		auto Handle = Create(Name);
		return GetImpl(Handle);
	}

	AssetHandle AssetCache::LoadAsset(StringView Name)
	{
		auto Asset = AssetLoader::Load(Name);
		if (!Asset)
			return GInvalidAssetHandle;

		auto Handle = NextHandle++;
		
		LoadedAssets.insert(std::make_pair(Handle, std::move(Asset)));

		return Handle;
	}
}
