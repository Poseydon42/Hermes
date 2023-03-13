#pragma once

#include "Core/Core.h"

namespace Hermes
{
	struct MeshComponent;
	struct TransformComponent;
	using ComponentID = uint8;
	using ComponentBitmask = size_t;

	namespace ComponentIDCounterInternal
	{
		inline ComponentID GNextComponentID = 0;
		
		inline ComponentID AllocateComponentID()
		{
			auto Result = GNextComponentID++;
			HERMES_ASSERT_LOG(GNextComponentID > Result, "Reached maximum component count for given component ID data type");

			return Result;
		}
	}

	template<typename ComponentType>
	ComponentID GetComponentID()
	{
		HERMES_ASSERT_LOG(false, "Trying to use undeclared comopnent");
	}

	template<typename ComponentType>
	ComponentBitmask GetComponentBitmask()
	{
		static auto Bitmask = static_cast<ComponentBitmask>(1) << GetComponentID<ComponentType>();
		HERMES_ASSERT_LOG(Bitmask > 0, "Reached maximum component count for given component bitmask data type");
		return Bitmask;
	}

	template<typename Last>
	ComponentBitmask GetComponentPackBitmask()
	{
		return GetComponentBitmask<Last>();
	}

	template<typename First, typename Second, typename... Rest>
	ComponentBitmask GetComponentPackBitmask()
	{
		return GetComponentBitmask<First>() | GetComponentPackBitmask<Second, Rest...>();
	}

#define HERMES_DECLARE_COMPONENT(Name)                                      \
	template<> inline HERMES_API ComponentID GetComponentID<Name>()                \
	{                                                                       \
		static auto ID = ::Hermes::ComponentIDCounterInternal::AllocateComponentID(); \
		return ID;                                                          \
	}

}
