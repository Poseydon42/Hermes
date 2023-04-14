#include "AssetCache.h"

#include "AssetSystem/AssetLoader.h"
#include "Core/Profiling.h"

namespace Hermes
{
	AssetHandle<Asset> AssetCache::GetImpl(const String& Name)
	{
		HERMES_PROFILE_FUNC();
		if (!LoadedAssets.contains(Name))
			return LoadAsset(Name);

		auto Result = LoadedAssets.at(Name);
		return Result;
	}

	AssetHandle<Asset> AssetCache::LoadAsset(const String& Name)
	{
		auto Asset = AssetLoader::Load(Name);

		if (Asset)
		{
			LoadedAssets.insert(std::make_pair(Name, Asset));
			return Asset;
		}
		return nullptr;
	}
}
