#include "EventQueue.h"

namespace Hermes
{
	void EventQueue::PushEvent(const IEvent& Event)
	{
		Queue.push(Event.Clone());
	}

	void EventQueue::ClearEventSubscribers(IEvent::EventType Type)
	{
		CallbackList[Type].Clear();
	}

	void EventQueue::Run()
	{
		while (!Queue.empty())
		{
			auto Event = Queue.front();
			Queue.pop();
			CallbackList[Event->GetType()].Invoke(*Event);
			// TODO : I don't really like that it uses heap each frame, maybe we have a way not to do so? E.g. create some custom allocator with predefined static storage
			delete Event;
		}
	}
}
