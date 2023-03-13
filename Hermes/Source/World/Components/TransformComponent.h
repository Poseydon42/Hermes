#pragma once

#include "Core/Core.h"
#include "Math/Transform.h"
#include "World/Component.h"

namespace Hermes
{
	struct HERMES_API TransformComponent
	{
		Transform Transform;
	};

	HERMES_DECLARE_COMPONENT(TransformComponent);
}
