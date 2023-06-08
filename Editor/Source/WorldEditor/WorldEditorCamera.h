#pragma once

#include "ApplicationCore/InputEngine.h"
#include "Core/Core.h"
#include "RenderingEngine/Scene/Camera.h"

namespace Hermes::Editor
{
	class APP_API WorldEditorCamera : public Camera
	{
	public:
		WorldEditorCamera(Vec3 InLocation, float InPitch, float InYaw);

		virtual Vec3 GetLocation() const override;

		virtual Vec3 GetDirection() const override;

		virtual float GetVerticalFOV() const override;

		virtual float GetNearZPlane() const override;

		virtual float GetFarZPlane() const override;

		virtual Mat4 GetViewMatrix() const override;

		virtual Mat4 GetProjectionMatrix(Vec2 ViewportDimensions) const override;

		virtual Frustum GetFrustum(Vec2 ViewportDimensions) const override;

		void Update(const InputEngine& Input, float DeltaTime);

	private:
		Vec3 Location = { 0.0f };
		float Pitch = 0.0f;
		float Yaw = 0.0f;

		static constexpr float VerticalFOV = Math::Pi / 3.0f;
		static constexpr float NearZ = 0.1f;
		static constexpr float FarZ = 1000.0f;

		static constexpr float MovementSpeed = 10.0f;
		static constexpr float RotationSpeed = 3.0f * Math::Pi;
		static constexpr float PitchLimit = Math::Radians(85.0f);

		Vec3 GetRightVector() const;
	};
}
