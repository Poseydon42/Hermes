#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	/**
	 * A dynamically linked library interface
	 */
	class HERMES_API IPlatformLibrary
	{
		MAKE_NON_COPYABLE(IPlatformLibrary)
	public:
		IPlatformLibrary() = default;

		virtual ~IPlatformLibrary() = default;
		IPlatformLibrary(IPlatformLibrary&&) = default;
		IPlatformLibrary& operator=(IPlatformLibrary&&) = default;
		
		static std::shared_ptr<IPlatformLibrary> Load(const String& Path);

		virtual void* GetSymbolAddress(const String& Name) = 0;

		virtual bool IsValid() = 0;
	};
}
