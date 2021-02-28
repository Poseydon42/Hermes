#pragma once

#include <unordered_map>
#include <queue>

#include "Core/Application/Event.h"
#include "Core/Delegate/Delegate.h"

namespace Hermes
{
	/**
	 * A delegated event queue
	 * All pending event will be handled during next Run() call
	 */
	class HERMES_API EventQueue
	{
	public:
		using CallbackDelegate = TMulticastDelegate<const IEvent&>;

		/**
		 * Pushes new event to the top of event queue
		 */
		void PushEvent(const IEvent& Event);

		/**
		 * Adds new listener to given event type
		 * Member function implementation
		 */
		template<class C, void(C::*Function)(const IEvent&)>
		void Subscribe(IEvent::EventType Type, C* Instance);

		/**
		 * Adds new listener to given event type
		 * Free function implementation
		 */
		template<void(*Function)(const IEvent&)>
		void Subscribe(IEvent::EventType Type);

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
		std::unordered_map<IEvent::EventType, CallbackDelegate> CallbackList;

		std::queue<IEvent*> Queue;
	};

	template<void(*Function)(const IEvent&)>
	void EventQueue::Subscribe(IEvent::EventType Type)
	{
		CallbackList[Type].Bind<Function>();
	}

	template<class C, void(C::* Function)(const IEvent&)>
	void EventQueue::Subscribe(IEvent::EventType Type, C* Instance)
	{
		CallbackList[Type].Bind<C, Function>(Instance)
	}

}
