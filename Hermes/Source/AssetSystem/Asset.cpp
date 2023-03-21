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


	bool Asset::IsValid() const
	{
		return false;
	}

	const Resource* Asset::GetResource() const
	{
		return nullptr;
	}
}
