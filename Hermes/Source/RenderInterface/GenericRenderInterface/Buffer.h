#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ResourceUsageType
		{
			VertexBuffer = 1 << 0,
			IndexBuffer = 1 << 1,
			UniformBuffer = 1 << 2,
			CopySource = 1 << 3,
			CopyDestination = 1 << 4,
			CPUAccessible = 1 << 5
		};

		ENUM_CLASS_OPERATORS(ResourceUsageType)

		/**
		 * Represents a resource that occupies some memory, has its format and can be read and written
		 */
		class HERMES_API Buffer
		{
		public:
			ADD_DEFAULT_CONSTRUCTOR(Buffer);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Buffer);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Buffer);
			MAKE_NON_COPYABLE(Buffer);

			/**
			 * Returns size of resource memory in bytes
			 */
			virtual size_t GetSize() const = 0;

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
