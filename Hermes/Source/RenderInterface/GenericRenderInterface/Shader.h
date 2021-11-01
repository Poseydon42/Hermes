#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ShaderType
		{
			VertexShader = 1,
			FragmentShader = 2,
			// TODO : expand
		};

		ENUM_CLASS_OPERATORS(ShaderType)
		
		class HERMES_API Shader
		{
			ADD_DEFAULT_CONSTRUCTOR(Shader)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Shader)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Shader)
			MAKE_NON_COPYABLE(Shader)

		public:
			virtual ShaderType GetType() const = 0;
		};

		class HERMES_API ShaderBase : public Shader
		{
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(ShaderBase)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(ShaderBase)
			MAKE_NON_COPYABLE(ShaderBase)
			
		public:
			explicit ShaderBase(ShaderType InType) : Type(InType) { }

			virtual ShaderType GetType() const override { return Type; }

		private:
			ShaderType Type;
		};
	}
}
