#pragma once

#include "Core/Core.h"
#include "Logging/Logger.h"

namespace Hermes
{
	using ComponentID = uint8;
	using ComponentBitmask = size_t;

	namespace ComponentIDCounterInternal
	{
		extern HERMES_API ComponentID GNextComponentID;
		
		HERMES_API ComponentID AllocateComponentID();
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

#ifdef HERMES_BUILD_ENGINE
#define HERMES_DECLARE_ENGINE_COMPONENT(Name)                                                \
	template<> inline HERMES_API ::Hermes::ComponentID Hermes::GetComponentID<Name>()        \
	{                                                                                        \
		static auto ID = ::Hermes::ComponentIDCounterInternal::AllocateComponentID();        \
		return ID;                                                                           \
	}                                                                                        
#elif defined(HERMES_BUILD_APPLICATION)
#define HERMES_DECLARE_ENGINE_COMPONENT(Name)                                          \
	template<> inline HERMES_API ::Hermes::ComponentID Hermes::GetComponentID<Name>();
#endif

#ifdef HERMES_BUILD_APPLICATION
#define HERMES_DECLARE_APPLICATION_COMPONENT(Name)                                    \
	template<> inline ::Hermes::ComponentID Hermes::GetComponentID<Name>()            \
	{                                                                                 \
		static auto ID = ::Hermes::ComponentIDCounterInternal::AllocateComponentID(); \
		return ID;                                                                    \
	}
#endif

}
