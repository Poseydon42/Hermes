#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	enum class AssetType : uint8
	{
		Image = 1,
		Mesh = 2,

		Material = 64,

		Invalid = 0xFF
	};

	class Resource;

	/*
	 * A generic asset representation
	 * Hermes assumes every file that is loaded by engine to be asset(with exceptions being dynamically linked libraries and configuration files)
	 * This class should be inherited for each known asset type to implement behaviour specific to it
	 */
	class HERMES_API Asset
	{
		MAKE_NON_COPYABLE(Asset)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Asset)
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Asset)
	public:
		Asset(String InName, AssetType InType);

		String GetName() const;

		AssetType GetType() const;

		virtual bool IsValid() const;

		virtual size_t GetMemorySize() const;

		virtual const Resource* GetResource() const;
		
		template<class AssetType>
		static const AssetType& As(const Asset& From);

		template<class AssetType>
		static std::unique_ptr<AssetType> As(std::unique_ptr<Asset> From);

	private:
		String Name;
		AssetType Type;
	};
}

#define DEFINE_ASSET_TYPE(Type)                                                                        \
	template<>                                                                                         \
	HERMES_API const Type##Asset& Asset::As<Type##Asset>(const Asset& From)                            \
	{                                                                                                  \
		HERMES_ASSERT(From.GetType() == AssetType::Type);                                              \
		return static_cast<const Type##Asset&>(From);                                                  \
	}                                                                                                  \
	template<>                                                                                         \
	HERMES_API std::unique_ptr<Type##Asset> Asset::As<Type##Asset>(std::unique_ptr<Asset> From)        \
	{                                                                                                  \
		HERMES_ASSERT(From->GetType() == AssetType::Type);                                             \
		return std::unique_ptr<Type##Asset>(static_cast<Type##Asset*>(From.release()));                \
	}                                                                                                  \
