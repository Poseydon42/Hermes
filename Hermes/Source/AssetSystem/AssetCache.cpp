#include "AssetCache.h"

#include "AssetSystem/AssetLoader.h"
#include "Core/Profiling.h"

namespace Hermes
{
	HERMES_API AssetHandle AssetHandle::InvalidHandle = AssetHandle(InvalidID, AssetType::Invalid);

	AssetHandle::AssetHandle(HandleIDType InHandleID, AssetType InType)
		: HandleID(InHandleID)
		, Type(InType)
	{
	}

	bool operator==(const AssetHandle& Left, const AssetHandle& Right)
	{
		return Left.HandleID == Right.HandleID && Left.Type == Right.Type;
	}

	size_t AssetHandleHasher::operator()(const AssetHandle& Value) const
	{
		return (static_cast<size_t>(Value.HandleID) << 32 | static_cast<std::underlying_type_t<decltype(Value.Type)>>(Value.Type));
	}

	AssetType AssetHandle::GetType() const
	{
		return Type;
	}

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

		if (Asset->GetType() != Handle.GetType())
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
			return AssetHandle::InvalidHandle;

		auto Handle = AssetHandle(NextID++, Asset->GetType());
		
		LoadedAssets.insert(std::make_pair(Handle, std::move(Asset)));

		return Handle;
	}
}
