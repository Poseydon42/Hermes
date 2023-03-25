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

		AssetHandle Create(StringView AssetName);

		template<class AssetClass>
		requires (std::derived_from<AssetClass, Asset>)
		std::optional<const AssetClass*> Get(AssetHandle Handle);

		// FIXME: at some point we would probably want to remove this function and make every asset be acquired via a handle
		template<class AssetClass>
		requires (std::derived_from<AssetClass, Asset>)
		std::optional<const AssetClass*> Get(StringView Name);

	private:
		AssetHandle NextHandle = GInvalidAssetHandle + 1;
		std::unordered_map<AssetHandle, std::unique_ptr<Asset>> LoadedAssets;

		std::optional<const Asset*> GetImpl(AssetHandle Handle);
		std::optional<const Asset*> GetImpl(StringView Name);

		AssetHandle LoadAsset(StringView Name);
	};

	template<class AssetClass>
	requires (std::derived_from<AssetClass, Asset>)
	std::optional<const AssetClass*> AssetCache::Get(AssetHandle Handle)
	{
		auto MaybeAsset = GetImpl(Handle);

		if (!MaybeAsset.has_value())
			return {};

		return &Asset::As<AssetClass>(*MaybeAsset.value());
	}

	template<class AssetClass>
	requires (std::derived_from<AssetClass, Asset>)
	std::optional<const AssetClass*> AssetCache::Get(StringView Name)
	{
		auto MaybeAsset = GetImpl(Name);

		if (!MaybeAsset.has_value())
			return {};

		return &Asset::As<AssetClass>(*MaybeAsset.value());
	}
}
