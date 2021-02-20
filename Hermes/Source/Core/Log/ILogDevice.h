#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/**
	 * Interface for a thing that can push log output into anything
	 * Each LogDevice instance is responsible only for pushing anything that it gets to its target
	 * No filtering of any kind should be performed
	 */
	class HERMES_API ILogDevice
	{
	public:
		virtual void Write(String Text) = 0;

		virtual void WriteLing(String Text) = 0;
	};
}