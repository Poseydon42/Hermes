#include "Asset.h"

namespace Hermes
{
	uint32 Asset::SNextID = 0;

	Asset::Asset(String InName, AssetType InType)
		: Name(std::move(InName))
		, Type(InType)
		, UniqueID(SNextID++)
	{
	}

	String Asset::GetName() const
	{
		return Name;
	}

	AssetType Asset::GetType() const
	{
		return Type;
	}

	AssetHandle<const Asset> Asset::GetSelfHandle() const
	{
		return shared_from_this();
	}

	AssetHandle<Asset> Asset::GetSelfHandle()
	{
		return shared_from_this();
	}

	uint32 Asset::GetUniqueID() const
	{
		return UniqueID;
	}

	bool Asset::IsValid() const
	{
		return false;
	}
}
