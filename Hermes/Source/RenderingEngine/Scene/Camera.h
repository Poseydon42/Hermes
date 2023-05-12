#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"

namespace Hermes
{
	struct Frustum;

	class HERMES_API Camera
	{
	public:
		ADD_DEFAULT_CONSTRUCTOR(Camera);
		ADD_DEFAULT_COPY_CONSTRUCTOR(Camera);
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Camera);
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Camera);

		virtual Vec3 GetLocation() const = 0;

		virtual Vec3 GetDirection() const = 0;

		virtual float GetVerticalFOV() const = 0;

		virtual float GetNearZPlane() const = 0;

		virtual float GetFarZPlane() const = 0;

		virtual Mat4 GetViewMatrix() const = 0;

		virtual Mat4 GetProjectionMatrix(Vec2 ViewportDimensions) const = 0;

		virtual Frustum GetFrustum(Vec2 ViewportDimensions) const = 0;
	};
}
