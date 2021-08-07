#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ResourceType
		{
			Unknown,
			Buffer
			// TODO : expand(texture, sampler etc.)
		};

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
		class HERMES_API Resource
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

		/**
		 * Class that implements some Resource's boilerplate code similar for all graphic APIs
		 */
		class HERMES_API ResourceBase : public Resource
		{
		public:
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(ResourceBase);
			MAKE_NON_COPYABLE(ResourceBase);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(ResourceBase);

			ResourceBase(ResourceType InType = ResourceType::Unknown, size_t InSize = 0)  : Type(InType), Size(InSize) { }
			
			size_t GetSize() const override { return Size; }
			ResourceType GetResourceType() const override { return Type; }
		
		private:
			ResourceType Type;
			size_t Size;
		};
	}
}
