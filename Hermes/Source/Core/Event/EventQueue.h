#pragma once

#include <functional>
#include <unordered_map>
#include <queue>

#include "Core/Event/Event.h"

namespace Hermes
{
	/**
	 * A delegated event queue
	 * All pending event will be handled during next Run() call
	 */
	class HERMES_API EventQueue
	{
	public:
		using CallbackFunctionPrototype = void(const IEvent&);

		/**
		 * Pushes new event to the top of event queue
		 */
		void PushEvent(const IEvent& Event);
		
		/**
		 * Adds new listener to given event type
		 * Free function implementation
		 */
		void Subscribe(IEvent::EventType Type, std::function<CallbackFunctionPrototype> Callback);

		/**
		 * Removes all subscribers of given event type
		 * NOTE : in current delegate implementation we don't have ability to compare delegates, so we aren't able to implement removing of single subscriber
		 */
		void ClearEventSubscribers(IEvent::EventType Type);

		/**
		 * Processes all pending events
		 */
		void Run();
	private:
		std::unordered_multimap<IEvent::EventType, std::function<CallbackFunctionPrototype>> Subscribers;

		std::queue<IEvent*> Queue;
	};
}
