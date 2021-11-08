#pragma once

#include "Core/Core.h"
#include "RenderingEngine/Scene/SceneProxies.h"

namespace Hermes
{
	/*
	 * NOTE : this all is 'the beginning' of a long looong journey
	 * This *will* be rewritten many times and currently is done only for testing
	 * and bootstrapping purposes
	 *
	 * A subset of drawable world elements that are currently in the camera frustum and
	 * thus should be displayed on the screen.
	 */
	class HERMES_API Scene
	{
	public:
		void AddMesh(MeshProxy Proxy);

		void UpdateCameraTransform(Vec3 Position, float Pitch, float Yaw);

		Mat4 GetViewMatrix() const;

		const std::vector<MeshProxy>& GetMeshes() const;

	private:
		std::vector<MeshProxy> Meshes;
		Vec3 CameraPosition;
		Vec3 CameraDirection;
	};
}