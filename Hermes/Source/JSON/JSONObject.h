#pragma once

#include <unordered_map>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "JSON/JSONValue.h"

namespace Hermes
{
	class HERMES_API JSONObject
	{
		MAKE_NON_COPYABLE(JSONObject)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(JSONObject)
		ADD_DEFAULT_DESTRUCTOR(JSONObject)

		using PropertiesContainerType = std::unordered_map<String, JSONValue>;
		using PropertiesConstIterator = PropertiesContainerType::const_iterator;

	public:
		JSONObject() = default;

		explicit JSONObject(PropertiesContainerType InProperties);

		size_t GetNumberOfProperties() const;

		bool Contains(const String& Name) const;
		
		const JSONValue& Get(const String& Name) const;

		const JSONValue& operator[](const String& Name) const;

		PropertiesConstIterator begin() const;
		PropertiesConstIterator end() const;
		
	private:
		PropertiesContainerType Properties;

		friend class JSONParser;
	};
}