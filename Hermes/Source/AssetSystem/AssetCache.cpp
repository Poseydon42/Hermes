#include "AssetCache.h"

#include "AssetSystem/AssetLoader.h"

namespace Hermes
{
	std::optional<const Asset*> AssetCache::GetImpl(const String& Name)
	{
		if (LoadedAssets.contains(Name))
			return LoadedAssets.at(Name).Asset.get();

		return LoadAsset(Name);
	}

	std::optional<const Asset*> AssetCache::LoadAsset(const String& Name)
	{
		HERMES_ASSERT(!LoadedAssets.contains(Name));

		auto Asset = AssetLoader::Load(Name);
		if (!Asset)
			return {};

		auto Result = Asset.get();

		CacheEntry NewEntry = { std::move(Asset), 0 };
		LoadedAssets.insert(std::make_pair(Name, std::move(NewEntry)));

		return Result;
	}
}
