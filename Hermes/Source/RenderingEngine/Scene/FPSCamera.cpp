#include "FPSCamera.h"

namespace Hermes
{
	FPSCamera::FPSCamera(
		Vec3 InLocation, float InPitch, float InYaw,
		float InMovementSpeed, float InRotationSpeed,
		float InVFOV, Vec2 ViewportDimensions, bool InIsYInverted)

		: Location(InLocation)
		  , Pitch(InPitch)
		  , Yaw(InYaw)
		  , MovementSpeed(InMovementSpeed)
		  , RotationSpeed(InRotationSpeed)
		  , VerticalFOV(InVFOV)
		  , AspectRatio(ViewportDimensions.X / ViewportDimensions.Y)
		  , IsYInverted(InIsYInverted)
	{
	}

	Vec3 FPSCamera::GetLocation() const
	{
		return Location;
	}

	Vec3 FPSCamera::GetDirection() const
	{
		return Direction;
	}

	float FPSCamera::GetVerticalFOV() const
	{
		return VerticalFOV;
	}

	void FPSCamera::UpdateViewportDimensions(Vec2 NewDimensions)
	{
		AspectRatio = NewDimensions.X / NewDimensions.Y;
	}

	Mat4 FPSCamera::GetViewMatrix() const
	{
		auto UpVector = (Direction ^ RightVector).Normalize();
		return Mat4::LookAt(Location, Direction, UpVector);
	}

	Mat4 FPSCamera::GetProjectionMatrix() const
	{
		return Mat4::Perspective(Math::Radians(VerticalFOV), AspectRatio, NearPlane, FarPlane);;
	}
	
	void FPSCamera::ApplyMovementInput(Vec2 Input, float DeltaTime)
	{
		Input *= DeltaTime;

		Vec3 DeltaLocation = Direction * Input.X + RightVector * Input.Y;
		DeltaLocation = DeltaLocation.SafeNormalize() * MovementSpeed;

		Location += DeltaLocation;
	}

	void FPSCamera::ApplyRotationInput(Vec2 Input, float DeltaTime)
	{
		Input *= DeltaTime;
		float DeltaPitch = Input.Y * RotationSpeed * (IsYInverted ? -1.f : 1.f);
		float DeltaYaw = Input.X * RotationSpeed;

		Pitch = Math::Clamp(Pitch + DeltaPitch, -PitchRotationLimit, PitchRotationLimit);
		Yaw += DeltaYaw;
		Yaw = fmod(Yaw, 2 * Math::Pi);
		if (Yaw > Math::Pi)
			Yaw = -(2 * Math::Pi - Yaw);
		if (Yaw < -Math::Pi)
			Yaw = 2 * Math::Pi + Yaw;

		Direction.X = Math::Sin(Yaw) * Math::Cos(Pitch);
		Direction.Y = Math::Sin(Pitch);
		Direction.Z = Math::Cos(Yaw) * Math::Cos(Pitch);
		Direction.Normalize();

		Vec3 GlobalUp = { 0.0f, 1.0f, 0.0f };
		RightVector = (GlobalUp ^ Direction).Normalize();
	}

	void FPSCamera::UpdateVerticalFOV(float NewFOV)
	{
		VerticalFOV = NewFOV;
	}

	void FPSCamera::SetYInversion(bool NewIsInverted)
	{
		IsYInverted = NewIsInverted;
	}
}
