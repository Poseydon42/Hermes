#include "JSONObject.h"

namespace Hermes
{
	JSONObject::JSONObject(PropertiesContainerType InProperties)
		: Properties(std::move(InProperties))
	{
	}

	size_t JSONObject::GetNumberOfProperties() const
	{
		return Properties.size();
	}

	bool JSONObject::Contains(const String& Name) const
	{
		return Properties.contains(Name);
	}

	const JSONValue& JSONObject::Get(const String& Name) const
	{
		return Properties.at(Name);
	}

	const JSONValue& JSONObject::operator[](const String& Name) const
	{
		return Get(Name);
	}

	JSONObject::PropertiesConstIterator JSONObject::begin() const
	{
		return Properties.begin();
	}

	JSONObject::PropertiesConstIterator JSONObject::end() const
	{
		return Properties.end();
	}
}
