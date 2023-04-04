#include "EventQueue.h"

namespace Hermes
{
	void EventQueue::PushEvent(const IEvent& Event)
	{
		Queue.push(Event.Clone());
	}

	void EventQueue::Subscribe(IEvent::EventType Type, std::function<CallbackFunctionPrototype> Callback)
	{
		Subscribers.emplace(Type, std::move(Callback));
	}

	void EventQueue::ClearEventSubscribers(IEvent::EventType Type)
	{
		for (auto Iterator = Subscribers.find(Type); Iterator != Subscribers.end(); ++Iterator)
		{
			Subscribers.erase(Iterator);
		}
	}

	void EventQueue::Run()
	{
		while (!Queue.empty())
		{
			auto Event = Queue.front();
			Queue.pop();

			for (auto Iterator = Subscribers.find(Event->GetType()); Iterator != Subscribers.end(); ++Iterator)
			{
				Iterator->second(*Event);
			}
			delete Event;
		}
	}
}
