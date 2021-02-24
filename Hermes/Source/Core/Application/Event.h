#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/**
	 * A single event that needs to be handled
	 * Must be copyable
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

#define EVENT_BODY() \
public: \
inline EventType GetType() const override { return Type; } \
static inline EventType GetStaticType() { return Type; } \
private: \
	static constexpr EventType Type = __COUNTER__;
