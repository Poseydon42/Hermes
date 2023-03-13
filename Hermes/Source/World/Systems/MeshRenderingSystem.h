#pragma once

#include "Core/Core.h"
#include "World/System.h"

namespace Hermes
{
	class HERMES_API MeshRenderingSystem : public ISystem
	{
	public:
		virtual void Run(World& World, Scene& Scene, float DeltaTime) const override;
	};
}