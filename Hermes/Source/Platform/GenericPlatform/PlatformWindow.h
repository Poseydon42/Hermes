#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyable.h"
#include "Math/Vector2.h"

namespace Hermes
{
	class EventQueue;
	
	class HERMES_API IPlatformWindow : public INonCopyable
	{
	public:
		virtual ~IPlatformWindow() = default;

		virtual void UpdateName(const String& NewName) = 0;

		virtual const String& GetName() const = 0;

		virtual bool ToggleFullscreen(bool Enabled) = 0;

		virtual bool UpdateVisibility(bool Visible) = 0;

		virtual bool Resize(Vec2i NewSize) = 0;

		virtual Vec2i GetSize() const = 0;

		virtual bool IsValid() const = 0;

		virtual std::weak_ptr<EventQueue> WindowQueue() = 0;

		static std::shared_ptr<IPlatformWindow> CreatePlatformWindow(const String& Name, Vec2i Size);
	};
}
