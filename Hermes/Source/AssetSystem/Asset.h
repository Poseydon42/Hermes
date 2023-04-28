#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	class Asset;

	enum class AssetType : uint8
	{
		Texture2D = 1,
		Mesh = 2,
		Font = 3,

		Material = 64,
		MaterialInstance = 65,

		Invalid = 0xFF
	};

	template<typename AssetClass>
	using AssetHandle = std::shared_ptr<AssetClass>;

	template<typename To, typename From>
	requires (std::derived_from<To, Asset> && std::derived_from<From, Asset>)
	AssetHandle<To> AssetCast(AssetHandle<From> Asset)
	{
		HERMES_ASSERT(Asset->GetType() == To::GetStaticType());
		return std::static_pointer_cast<To>(Asset);
	}

	template<typename To, typename From>
	requires (std::derived_from<To, Asset> && std::derived_from<From, Asset>)
	AssetHandle<const To> AssetCast(AssetHandle<const From> Asset)
	{
		HERMES_ASSERT(Asset->GetType() == To::GetStaticType());
		return std::static_pointer_cast<To>(Asset);
	}

	/*
	 * A generic asset representation
	 * Hermes assumes every file that is loaded by engine to be asset(with exceptions being dynamically linked libraries and configuration files)
	 * This class should be inherited for each known asset type to implement behaviour specific to it
	 */
	class HERMES_API Asset : public std::enable_shared_from_this<Asset>
	{
		MAKE_NON_COPYABLE(Asset)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Asset)
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Asset)
	public:
		Asset(String InName, AssetType InType);

		String GetName() const;

		AssetType GetType() const;

		AssetHandle<const Asset> GetSelfHandle() const;
		AssetHandle<Asset> GetSelfHandle();

		uint32 GetUniqueID() const;

		virtual bool IsValid() const;

	private:
		String Name;
		AssetType Type;

		uint32 UniqueID;

		static uint32 SNextID;
	};
}

#define HERMES_DECLARE_ASSET(Type)          \
	public:                                 \
	inline static AssetType GetStaticType() \
	{                                       \
		return AssetType::Type;             \
	}
