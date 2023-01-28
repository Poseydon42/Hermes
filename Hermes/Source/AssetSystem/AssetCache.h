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

		template<class AssetClass>
		requires (std::derived_from<AssetClass, Asset>)
		std::optional<const AssetClass*> Get(const String& Name);

		void Release(const String& Name);

	private:
		struct CacheEntry
		{
			std::unique_ptr<Asset> Asset;
			uint32 UseCount = 0;
		};

		std::optional<const Asset*> GetImpl(const String& Name);

		std::optional<const Asset*> LoadAsset(const String& Name);

		std::unordered_map<String, CacheEntry> LoadedAssets;
	};

	template<class AssetClass>
	requires (std::derived_from<AssetClass, Asset>)
	std::optional<const AssetClass*> AssetCache::Get(const String& Name)
	{
		auto MaybeAsset = GetImpl(Name);

		if (!MaybeAsset.has_value())
			return {};

		return &Asset::As<AssetClass>(*MaybeAsset.value());
	}
}
