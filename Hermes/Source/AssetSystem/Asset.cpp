#include "Asset.h"

namespace Hermes
{
	Asset::Asset(String InName, AssetType InType, AssetHandle InSelfHandle)
		: Name(std::move(InName))
		, Type(InType)
		, SelfHandle(InSelfHandle)
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

	AssetHandle Asset::GetSelfHandle() const
	{
		return SelfHandle;
	}

	bool Asset::IsValid() const
	{
		return false;
	}
}
