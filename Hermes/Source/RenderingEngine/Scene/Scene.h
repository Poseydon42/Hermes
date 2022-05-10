#pragma once

#include "Core/Core.h"
#include "RenderingEngine/Scene/SceneProxies.h"

namespace Hermes
{
	class Camera;

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

		void AddPointLight(PointLightProxy Proxy);

		void ChangeActiveCamera(std::shared_ptr<Camera> NewCamera);

		Camera& GetActiveCamera() const;

		const std::vector<MeshProxy>& GetMeshes() const;

		const std::vector<PointLightProxy>& GetPointLights() const;

	private:
		std::vector<MeshProxy> Meshes;
		std::vector<PointLightProxy> PointLights;

		std::shared_ptr<Camera> ActiveCamera;
	};
}
