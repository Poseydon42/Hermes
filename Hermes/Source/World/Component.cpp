#include "Component.h"

namespace Hermes::ComponentIDCounterInternal
{
	ComponentID GNextComponentID = 0;

	ComponentID AllocateComponentID()
	{
		auto Result = GNextComponentID++;
		HERMES_ASSERT_LOG(GNextComponentID > Result, "Reached maximum component count for given component ID data type");

		return Result;
	}
}