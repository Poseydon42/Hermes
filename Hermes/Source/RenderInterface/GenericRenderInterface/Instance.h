#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	namespace RenderInterface
	{
		/**
		 * This is a main access point to all rendering API functionality
		 * It is responsible for enumerating available devices and creating them
		 */
		class HERMES_API Instance
		{
			MAKE_NON_COPYABLE(Instance)
		
		public:
			Instance() = default;
			
			virtual ~Instance() = default;
			Instance(Instance&& Other) = default;
			Instance& operator=(Instance&& Other) = default;

			virtual std::vector<DeviceProperties> EnumerateAvailableDevices() = 0;
		};
	}
}

