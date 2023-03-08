#include "Resource.h"

namespace Hermes
{
	Resource::Resource(String InName, ResourceType InType)
		: Name(std::move(InName))
		, Type(InType)
	{
	}

	ResourceType Resource::GetType() const
	{
		return Type;
	}

	StringView Resource::GetName() const
	{
		return Name;
	}
}
