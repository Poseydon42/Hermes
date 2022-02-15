﻿#pragma once

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class CommandBuffer;
	}

	class Scene;
	struct Source;
	struct Drain;

	struct PassDesc
	{
		/*
		 * void Callback(RenderInterface::CommandBuffer& TargetCommandBuffer, const Scene& Scene, bool ResourcesWereChanged)
		 */
		using RenderPassCallbackType = TDelegate<void, RenderInterface::CommandBuffer&, const Scene&, bool>;
		
		std::vector<Drain> Drains;
		std::vector<Source> Sources;
		RenderPassCallbackType Callback;
	};
}
