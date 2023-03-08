#include "Asset.h"

namespace Hermes
{
	Asset::Asset(const String& InName, AssetType InType)
		: Name(InName)
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

	size_t Asset::GetMemorySize() const
	{
		return 0;
	}

	const Resource* Asset::GetResource() const
	{
		return nullptr;
	}
}
