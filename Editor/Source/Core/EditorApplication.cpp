#include "EditorApplication.h"

#include "ApplicationCore/GameLoop.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Renderer.h"
#include "VirtualFilesystem/DirectoryFSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "World/Components/DirectionalLightComponent.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/TagComponent.h"
#include "World/Components/TransformComponent.h"

namespace Hermes::Editor
{
	static void BuildDebugWorld(World& World)
	{
		auto SphereMesh = AssetLoader::Load<Mesh>("/Editor/Debug/sphere");
		HERMES_ASSERT(SphereMesh);

		auto SphereMaterial = AssetLoader::Load<Material>("/Materials/m_pbr");
		HERMES_ASSERT(SphereMaterial);

		auto SphereMaterialInstance = MaterialInstance::Create("SphereMaterialInstance", SphereMaterial);
		HERMES_ASSERT(SphereMaterialInstance);

		auto AlbedoTexture = Texture2D::Create("SphereAlbedoTexture", Vec2ui(2), ImageFormat::RGBA, 1, Vec4(0.74f, 0.1f, 0.25f, 1.0f));
		auto NormalTexture = Texture2D::Create("SphereNormalTexture", Vec2ui(2), ImageFormat::RGBA, 1, Vec4(0.5f, 0.5f, 1.0f, 0.0f));
		auto RoughnessTexture = Texture2D::Create("SphereRoughnessTexture", Vec2ui(2), ImageFormat::R, 1, Vec4(1.0f, 0.0f, 0.0f, 0.0f));
		auto MetallicTexture = Texture2D::Create("SphereMetallicTexture", Vec2ui(2), ImageFormat::R, 1, Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		SphereMaterialInstance->SetTextureProperty("u_AlbedoTexture", AlbedoTexture, ColorSpace::SRGB);
		SphereMaterialInstance->SetTextureProperty("u_NormalTexture", NormalTexture, ColorSpace::Linear);
		SphereMaterialInstance->SetTextureProperty("u_RoughnessTexture", RoughnessTexture, ColorSpace::Linear);
		SphereMaterialInstance->SetTextureProperty("u_MetallicTexture", MetallicTexture, ColorSpace::Linear);

		auto SphereEntity = World.CreateEntity();
		World.AddComponent<TransformComponent>(SphereEntity);
		World.AddComponent<MeshComponent>(SphereEntity) = { SphereMesh, SphereMaterialInstance };
		World.AddComponent<TagComponent>(SphereEntity).Tag = "Sphere";

		auto DirectionalLightEntity = World.CreateEntity();
		auto& DirectionalLight = World.AddComponent<DirectionalLightComponent>(DirectionalLightEntity);
		DirectionalLight.Color = Vec3(1.0f, 1.0f, 1.0f);
		DirectionalLight.Intensity = 50.0f;
		DirectionalLight.Direction = Vec3(-1.0f).Normalized();
		World.AddComponent<TagComponent>(DirectionalLightEntity).Tag = "DirectionalLight";
	}

	bool EditorApplication::EarlyInit()
	{
		VirtualFilesystem::Mount("/Editor", MountMode::ReadOnly, 1, std::make_unique<DirectoryFSDevice>("Editor/Files"));
		VirtualFilesystem::Mount("/", MountMode::ReadWrite, 2, std::make_unique<DirectoryFSDevice>("Editor/Temp"));

		return true;
	}

	bool EditorApplication::Init()
	{
		HERMES_LOG_INFO("Initializing the editor");

		Viewport = WorldEditorViewport::Create();
		GGameLoop->SetRootWidget(Viewport);

		Camera = std::make_shared<WorldEditorCamera>(Vec3(0.0f), 0.0f, 0.0f);
		GGameLoop->OverrideCamera(Camera);

		BuildDebugWorld(GGameLoop->GetWorld());

		return true;
	}

	void EditorApplication::Run(float DeltaTime)
	{
		Camera->Update(GGameLoop->GetInputEngine(), DeltaTime);
	}

	void EditorApplication::Shutdown()
	{
		HERMES_LOG_INFO("Shutting down the editor");
	}
	
	extern "C" APP_API IApplication * CreateApplicationInstance()
	{
		auto Application = new EditorApplication;
		return Application;
	}
}
