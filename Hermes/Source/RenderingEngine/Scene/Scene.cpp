﻿#include "Scene.h"

namespace Hermes
{
	void Scene::AddMesh(MeshProxy Proxy)
	{
		Meshes.push_back(std::move(Proxy));
	}

	void Scene::UpdateCameraTransform(Vec3 Position, float Pitch, float Yaw)
	{
		CameraPosition = Position;
		CameraDirection.X = -Math::Sin(Math::Radians(Yaw));
		CameraDirection.Y = Math::Sin(Math::Radians(Pitch));
		CameraDirection.Z = Math::Cos(Math::Radians(Yaw)) * Math::Cos(Math::Radians(Pitch));
		CameraDirection.Normalize();
	}

	Mat4 Scene::GetViewMatrix() const
	{
		Vec3 GlobalUp = { 0, 1, 0 };
		Vec3 CameraRight = (GlobalUp ^ CameraDirection).Normalize();
		Vec3 CameraUp = (CameraDirection ^ CameraRight).Normalize();
		return Mat4::LookAt(CameraPosition, CameraDirection, CameraUp);
	}

	const std::vector<MeshProxy>& Scene::GetMeshes() const
	{
		return Meshes;
	}
}
