#include "Asset.h"

namespace Hermes
{
	Asset::Asset(String InName, AssetType InType)
		: Name(std::move(InName))
		, Type(InType)
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


	bool Asset::IsValid() const
	{
		return false;
	}
}
