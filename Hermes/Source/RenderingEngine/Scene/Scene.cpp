#include "Scene.h"

#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/ImageAsset.h"

namespace Hermes
{
	Scene::Scene()
	{
		auto RawReflectionEnvmapAsset = Asset::As<ImageAsset>(
			AssetLoader::Load(L"Textures/default_envmap_reflection"));
		HERMES_ASSERT_LOG(RawReflectionEnvmapAsset, L"Failed to load reflection envmap.");
		auto RawReflectionEnvmapTexture = Texture::CreateFromAsset(*RawReflectionEnvmapAsset, false);
		ReflectionEnvmap = CubemapTexture::CreateFromEquirectangularTexture(*RawReflectionEnvmapTexture,
		                                                                    RenderInterface::DataFormat::R16G16B16A16SignedFloat,
		                                                                    false);
	}

	void Scene::AddMesh(MeshProxy Proxy)
	{
		Meshes.push_back(std::move(Proxy));
	}

	const std::vector<PointLightProxy>& Scene::GetPointLights() const
	{
		return PointLights;
	}

	const CubemapTexture& Scene::GetReflectionEnvmap() const
	{
		return *ReflectionEnvmap;
	}

	const std::vector<MeshProxy>& Scene::GetMeshes() const
	{
		return Meshes;
	}

	void Scene::AddPointLight(PointLightProxy Proxy)
	{
		PointLights.push_back(Proxy);
	}

	void Scene::ChangeActiveCamera(std::shared_ptr<Camera> NewCamera)
	{
		ActiveCamera = std::move(NewCamera);
	}

	Camera& Scene::GetActiveCamera() const
	{
		return *ActiveCamera;
	}
}
