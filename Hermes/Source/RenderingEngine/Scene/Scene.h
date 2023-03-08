#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/Scene/GeometryList.h"
#include "RenderingEngine/Scene/SceneNode.h"
#include "RenderingEngine/Resource/TextureResource.h"

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
		MAKE_NON_COPYABLE(Scene)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Scene)
		ADD_DEFAULT_DESTRUCTOR(Scene)

	public:
		Scene();

		SceneNode& GetRootNode();
		const SceneNode& GetRootNode() const;

		void Reset();

		void ChangeActiveCamera(std::shared_ptr<Camera> NewCamera);

		Camera& GetActiveCamera() const;

		GeometryList BakeGeometryList() const;

		const TextureCubeResource& GetReflectionEnvmap() const;
		const TextureCubeResource& GetIrradianceEnvmap() const;
		const TextureCubeResource& GetSpecularEnvmap() const;

	private:
		SceneNode RootNode = SceneNode(SceneNodeType::None);

		std::unique_ptr<TextureCubeResource> ReflectionEnvmap;
		std::unique_ptr<TextureCubeResource> IrradianceEnvmap;
		std::unique_ptr<TextureCubeResource> SpecularEnvmap;

		std::shared_ptr<Camera> ActiveCamera;
	};
}
