#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/**
	 * A single event that needs to be handled
	 */
	class HERMES_API Event
	{
	public:
		inline Event(const String& EventName, uint32 EventType) : Name(EventName), Type(EventType) { }

		inline const String& GetName() const { return Name;  }

		inline uint32 GetType() const { return Type; }

		virtual String ToString() const = 0;
	private:
		String Name;

		uint32 Type;
	};
}