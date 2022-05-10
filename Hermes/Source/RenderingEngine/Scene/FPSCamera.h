#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"
#include "Math/Vector2.h"
#include "RenderingEngine/Scene/Camera.h"

namespace Hermes
{
	class HERMES_API FPSCamera : public Camera
	{
	public:
		FPSCamera(
			Vec3 InLocation, float InPitch, 
			float InYaw, float InMovementSpeed, 
			float InRotationSpeed, float InVFOV,
			Vec2 ViewportDimensions, bool InIsYInverted);

		virtual Vec3 GetLocation() const override;

		virtual Vec3 GetDirection() const override;

		virtual float GetVerticalFOV() const override;

		virtual void UpdateViewportDimensions(Vec2 NewDimensions) override;

		virtual Mat4 BuildViewProjectionMatrix() const override;

		void ApplyMovementInput(Vec2 Input, float DeltaTime);

		void ApplyRotationInput(Vec2 Input, float DeltaTime);

		void UpdateVerticalFOV(float NewFOV);

		void SetYInversion(bool NewIsInverted);

	private:
		Vec3 Location;

		float Pitch, Yaw;

		Vec3 Direction = Vec3(0), RightVector = Vec3(0);

		float MovementSpeed, RotationSpeed;

		float VerticalFOV, AspectRatio;

		bool IsYInverted;

		static constexpr float PitchRotationLimit = 0.95f * Math::HalfPi;

		// TODO : move these values into settings
		static constexpr float NearPlane = 0.1f;
		static constexpr float FarPlane = 1000.0f;
	};
}
