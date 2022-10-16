#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/MeshAsset.h"
#include "ApplicationCore/InputEngine.h"
#include "Core/Event/Event.h"
#include "Math/Vector.h"
#include "RenderingEngine/Scene/FPSCamera.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderingEngine/Material/Material.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Texture.h"

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto SphereMesh = Hermes::Asset::As<Hermes::MeshAsset>(Hermes::AssetLoader::Load(L"sphere"));
		auto BoundingVolume = SphereMesh->GetBoundingVolume();
		auto SphereMeshBuffer = Hermes::MeshBuffer::CreateFromAsset(SphereMesh);

		auto AlbedoTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_albedo"));
		auto NormalTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_normal"));
		auto MetallicTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_metallic"));
		auto RoughnessTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_roughness"));
		AlbedoTexture = Hermes::Texture::CreateFromAsset(*AlbedoTextureAsset, true);
		NormalTexture = Hermes::Texture::CreateFromAsset(*NormalTextureAsset, false);
		MetallicTexture = Hermes::Texture::CreateFromAsset(*MetallicTextureAsset, false);
		RoughnessTexture = Hermes::Texture::CreateFromAsset(*RoughnessTextureAsset, false);

		TestMaterial = Hermes::Material::Create(L"Shaders/Bin/forward_vert.glsl.spv",
		                                        L"Shaders/Bin/forward_frag.glsl.spv");
		TestMaterialInstance = TestMaterial->CreateInstance();
		TestMaterialInstance->SetTextureProperty(L"u_AlbedoTexture", *AlbedoTexture);
		TestMaterialInstance->SetTextureProperty(L"u_RoughnessTexture", *RoughnessTexture);
		TestMaterialInstance->SetTextureProperty(L"u_MetallicTexture", *MetallicTexture);
		TestMaterialInstance->SetTextureProperty(L"u_NormalTexture", *NormalTexture);

		static constexpr Hermes::int32 SphereCountInSingleDimension = 19;
		for (Hermes::int32 SphereX = -SphereCountInSingleDimension / 2;
		     SphereX <= SphereCountInSingleDimension / 2;
		     SphereX++)
		{
			for (Hermes::int32 SphereZ = -SphereCountInSingleDimension / 2;
			     SphereZ <= SphereCountInSingleDimension / 2;
			     SphereZ++)
			{
				static constexpr float DistanceBetweenSpheres = 10.0f;
				Hermes::Vec3 SphereLocation = {
					static_cast<float>(SphereX) * DistanceBetweenSpheres,
					0.0f,
					static_cast<float>(SphereZ) * DistanceBetweenSpheres
				};

				Hermes::MeshProxy SphereMeshProxy =
				{
					Hermes::Mat4::Translation(SphereLocation),
					BoundingVolume,
					SphereMeshBuffer,
					TestMaterialInstance
				};
				Hermes::GGameLoop->GetScene().AddMesh(SphereMeshProxy);
			}
		}

		Hermes::PointLightProxy PointLight = {};
		PointLight.Color = { 1.0f, 1.0f, 0.9f, 200.0f };
		PointLight.Position = { 0.0f, 3.0f, 3.0f, 0.0f };
		Hermes::GGameLoop->GetScene().AddPointLight(PointLight);

		Camera = std::make_unique<Hermes::FPSCamera>(
			Hermes::Vec3(0.0f), 0.0f, 0.0f, 0.5f, 25.0f, 50.0f,
			Hermes::Vec2(Hermes::Renderer::Get().GetSwapchain().GetSize()), true);
		Hermes::GGameLoop->GetScene().ChangeActiveCamera(Camera);

		Hermes::GGameLoop->GetInputEngine().GetEventQueue().Subscribe<SandboxApp, &SandboxApp::KeyEventHandler>(Hermes::KeyEvent::GetStaticType(), this);

		return true;
	}

	void Run(float DeltaTime) override
	{
		const auto& InputEngine = Hermes::GGameLoop->GetInputEngine();
		
		Hermes::Vec2 CameraMovementInput = {};
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::W))
			CameraMovementInput.X += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::S))
			CameraMovementInput.X -= 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::D))
			CameraMovementInput.Y += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::A))
			CameraMovementInput.Y -= 1.0f;

		Camera->ApplyMovementInput(CameraMovementInput, DeltaTime);
		Camera->ApplyRotationInput(InputEngine.GetDeltaMousePosition(), DeltaTime);

		if (AnisotropyChanged)
		{
			Hermes::GraphicsSettings Settings;
			Settings.AnisotropyLevel = AnisotropyEnabled ? 16.0f : 0.0f;
			Hermes::Renderer::Get().UpdateGraphicsSettings(Settings);
		}

		/* LIGHT DEBUG */
		float DeltaLightPower = 0.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::ArrowUp))
			DeltaLightPower += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::ArrowDown))
			DeltaLightPower -= 1.0f;
		DeltaLightPower *= DeltaTime * 200.0f;
		auto& Light = const_cast<Hermes::PointLightProxy&>(Hermes::GGameLoop->GetScene().GetPointLights()[0]);
		Light.Color.W += DeltaLightPower;
		Light.Color.W = Hermes::Math::Max(Light.Color.W, 0.0f);
	}

	void Shutdown() override
	{
	}

private:
	bool AnisotropyEnabled = false, AnisotropyChanged = false;
	std::shared_ptr<Hermes::FPSCamera> Camera;
	std::shared_ptr<Hermes::Material> TestMaterial;
	std::shared_ptr<Hermes::MaterialInstance> TestMaterialInstance;
	std::shared_ptr<Hermes::Texture> AlbedoTexture, NormalTexture, MetallicTexture, RoughnessTexture;

	void KeyEventHandler(const Hermes::IEvent& Event)
	{
		if (Event.GetType() == Hermes::KeyEvent::GetStaticType())
		{
			const auto& KeyEvent = static_cast<const Hermes::KeyEvent&>(Event);
			if (KeyEvent.GetEventType() == Hermes::KeyEventType::Pressed && KeyEvent.GetKeyCode() == Hermes::KeyCode::M)
			{
				AnisotropyChanged = true;
				AnisotropyEnabled = !AnisotropyEnabled;
			}
		}
	}
};

extern "C" APP_API Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
