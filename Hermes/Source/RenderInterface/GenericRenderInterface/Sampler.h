#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class AddressingMode
		{
			Repeat,
			RepeatMirrored,
			ClampToEdge,
		};

		enum class FilteringMode
		{
			Nearest,
			Linear
		};

		enum class CoordinateSystem
		{
			Normalized,
			Unnormalized
		};

		struct SamplerDescription
		{
			AddressingMode AddressingModeU;
			AddressingMode AddressingModeV;
			FilteringMode MinificationFilteringMode;
			FilteringMode MagnificationFilteringMode;
			CoordinateSystem CoordinateSystem;
			// TODO : mip levels
			// TODO : anisotropy
		};

		class HERMES_API Sampler
		{
			MAKE_NON_COPYABLE(Sampler)
			ADD_DEFAULT_CONSTRUCTOR(Sampler)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Sampler)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Sampler)

		};
	}
}
