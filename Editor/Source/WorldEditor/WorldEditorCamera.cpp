#include "WorldEditorCamera.h"

#include "Math/Frustum.h"

namespace Hermes::Editor
{
	WorldEditorCamera::WorldEditorCamera(Vec3 InLocation, float InPitch, float InYaw)
		: Location(InLocation)
		, Pitch(InPitch)
		, Yaw(InYaw)
	{
	}

	Vec3 WorldEditorCamera::GetLocation() const
	{
		return Location;
	}

	Vec3 WorldEditorCamera::GetDirection() const
	{
		Vec3 Result = { Math::Sin(Yaw) * Math::Cos(Pitch), Math::Sin(Pitch), Math::Cos(Yaw) * Math::Cos(Pitch) };
		return Result.Normalized();
	}

	float WorldEditorCamera::GetVerticalFOV() const
	{
		return VerticalFOV;
	}

	float WorldEditorCamera::GetNearZPlane() const
	{
		return NearZ;
	}

	float WorldEditorCamera::GetFarZPlane() const
	{
		return FarZ;
	}

	Mat4 WorldEditorCamera::GetViewMatrix() const
	{
		auto Direction = GetDirection();
		auto Right = GetRightVector();
		auto Up = Right.Cross(Direction).Normalized();
		
		return Mat4::LookAt(Location, Direction, Up);
	}

	Mat4 WorldEditorCamera::GetProjectionMatrix(Vec2 ViewportDimensions) const
	{
		return Mat4::Perspective(VerticalFOV, ViewportDimensions.X / ViewportDimensions.Y, NearZ, FarZ);
	}

	Frustum WorldEditorCamera::GetFrustum(Vec2 ViewportDimensions) const
	{
		auto Direction = GetDirection();
		auto Right = GetRightVector();
		float AspectRatio = ViewportDimensions.X / ViewportDimensions.Y;

		Frustum Result = {};

		auto UpVector = Right.Cross(Direction).Normalized();
		auto VectorToCenterOfFarPlane = Direction * FarZ;
		auto HalfVerticalSizeOfFarPlane = FarZ * Math::Tan(0.5f * VerticalFOV);
		auto HalfHorizontalSizeOfFarPlane = HalfVerticalSizeOfFarPlane * AspectRatio;

		Result.Near = Plane(Direction, Location + Direction * NearZ);
		Result.Far = Plane(-Direction, Location + VectorToCenterOfFarPlane);
		Result.Right = Plane(UpVector.Cross(VectorToCenterOfFarPlane + Right * HalfHorizontalSizeOfFarPlane), Location);
		Result.Left = Plane((VectorToCenterOfFarPlane - Right * HalfHorizontalSizeOfFarPlane).Cross(UpVector), Location);
		Result.Top = Plane((VectorToCenterOfFarPlane + UpVector * HalfVerticalSizeOfFarPlane).Cross(Right), Location);
		Result.Bottom = Plane(Right.Cross(VectorToCenterOfFarPlane - UpVector * HalfVerticalSizeOfFarPlane), Location);

		return Result;
	}

	void WorldEditorCamera::Update(const InputEngine& Input, float DeltaTime)
	{
		auto Forward = GetDirection();
		auto Right = GetRightVector();

		Vec3 DeltaLocation = {};
		if (Input.IsKeyPressed(KeyCode::W))
			DeltaLocation += Forward;
		if (Input.IsKeyPressed(KeyCode::S))
			DeltaLocation -= Forward;
		if (Input.IsKeyPressed(KeyCode::D))
			DeltaLocation += Right;
		if (Input.IsKeyPressed(KeyCode::A))
			DeltaLocation -= Right;
		DeltaLocation = DeltaLocation.SafeNormalized() * DeltaTime * MovementSpeed;
		Location += DeltaLocation;

		auto MouseDelta = Input.GetDeltaMousePosition();
		float DeltaPitch = -MouseDelta.Y * RotationSpeed * DeltaTime;
		float DeltaYaw = -MouseDelta.X * RotationSpeed * DeltaTime;

		Pitch = Math::Clamp(-PitchLimit, PitchLimit, Pitch + DeltaPitch);
		Yaw += DeltaYaw;
	}

	Vec3 WorldEditorCamera::GetRightVector() const
	{
		auto Direction = GetDirection();

		auto GlobalUp = Vec3(0.0f, 1.0f, 0.0f);
		auto Right = Direction.Cross(GlobalUp).Normalized();

		return Right;
	}
}
