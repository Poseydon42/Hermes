#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ResourceType
		{
			Buffer
			// TODO : expand(texture, sampler etc.)
		};

		/**
		 * Represents a resource that occupies some memory, has its format and can be read and written
		 */
		class Resource
		{
		public:
			ADD_DEFAULT_CONSTRUCTOR(Resource);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Resource);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Resource);
			MAKE_NON_COPYABLE(Resource);

			/**
			 * Returns size of resource memory in bytes
			 */
			virtual size_t GetSize() const = 0;
			
			virtual ResourceType GetResourceType() const = 0;

			/**
			 * Maps resource memory into application virtual memory space
			 */
			virtual void* Map() = 0;

			/**
			 * Unmaps resource memory from application virtual memory space
			 */
			virtual void Unmap() = 0;
		};
	}
}
