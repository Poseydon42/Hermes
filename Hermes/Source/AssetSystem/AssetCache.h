#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <unordered_map>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	class HERMES_API AssetCache
	{
		MAKE_NON_COPYABLE(AssetCache)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(AssetCache)
		ADD_DEFAULT_DESTRUCTOR(AssetCache)

	public:
		AssetCache() = default;

		template<typename AssetClass>
		requires (std::derived_from<AssetClass, Asset>)
		AssetHandle<AssetClass> Get(const String& AssetName);

	private:
		std::unordered_map<String, AssetHandle<Asset>> LoadedAssets;
		
		AssetHandle<Asset> GetImpl(const String& Name);

		AssetHandle<Asset> LoadAsset(const String& Name);
	};

	template<typename AssetClass>
	requires (std::derived_from<AssetClass, Asset>)
	AssetHandle<AssetClass> AssetCache::Get(const String& AssetName)
	{
		auto Result = GetImpl(AssetName);

		if (Result->GetType() != AssetClass::GetStaticType())
			return nullptr;

		return AssetCast<AssetClass>(Result);
	}

	template<>
	inline AssetHandle<Asset> AssetCache::Get(const String& Name)
	{
		return GetImpl(Name);
	}
}
