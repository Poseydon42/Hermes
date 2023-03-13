#pragma once

#include "Core/Core.h"

namespace Hermes
{
	class Scene;
	class World;

	class HERMES_API ISystem
	{
	public:
		virtual ~ISystem() = default;

		virtual void Run(World& World, Scene& Scene, float DeltaTime) const = 0;
	};
}