#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/**
	 * A single event that needs to be handled
	 */
	class HERMES_API IEvent
	{
	public:
		using EventType = uint32;

		virtual ~IEvent() = default;

		virtual EventType GetType() const = 0;

		virtual String ToString() const = 0;
	};
}
