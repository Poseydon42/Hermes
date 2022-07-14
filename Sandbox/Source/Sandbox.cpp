#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Core/Application/GameLoop.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/MeshAsset.h"
#include "Core/Application/InputEngine.h"
#include "Core/Application/Event.h"
#include "Math/Vector.h"
#include "RenderingEngine/Scene/FPSCamera.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto SphereMesh = Hermes::Asset::As<Hermes::MeshAsset>(Hermes::AssetLoader::Load(L"sphere"));
		auto SphereMeshBuffer = Hermes::MeshBuffer::CreateFromAsset(SphereMesh);
		auto AlbedoTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_albedo"));
		auto MetallicTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_metallic"));
		auto RoughnessTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_roughness"));

		auto AlbedoTexture = Hermes::Texture::CreateFromAsset(*AlbedoTextureAsset, true);
		auto MetallicTexture = Hermes::Texture::CreateFromAsset(*MetallicTextureAsset, false);
		auto RoughnessTexture = Hermes::Texture::CreateFromAsset(*RoughnessTextureAsset, false);
		std::shared_ptr<Hermes::Material> PBRMaterial = std::make_shared<Hermes::Material>(std::vector{ AlbedoTexture, RoughnessTexture, MetallicTexture });
		Hermes::MeshProxy SphereMeshProxy =
		{
			Hermes::Mat4::Translation(SphereLocation),
			*SphereMeshBuffer,
			PBRMaterial
		};
		Hermes::GGameLoop->GetScene().AddMesh(SphereMeshProxy);

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


		/* OBJECT ROTATION DEBUG */
		float DeltaAngle = 0.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::E))
			DeltaAngle += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::Q))
			DeltaAngle -= 1.0f;
		DeltaAngle *= DeltaTime * Hermes::Math::Pi / 3.0f;
		DebugObjectAngle += DeltaAngle;
		auto RotationMatrix = Hermes::Mat4::Rotation(Hermes::Vec3{0.0f, DebugObjectAngle, 0.0f});
		auto RotationMatrix4 = Hermes::Mat4(RotationMatrix);
		RotationMatrix4[3][3] = 1.0f;
		const_cast<Hermes::MeshProxy&>(Hermes::GGameLoop->GetScene().GetMeshes()[0]).TransformationMatrix =
			Hermes::Mat4::Translation(SphereLocation) * RotationMatrix4;
	}

	void Shutdown() override
	{
	}

private:
	bool AnisotropyEnabled = false, AnisotropyChanged = false;
	std::shared_ptr<Hermes::FPSCamera> Camera;
	const Hermes::Vec3 SphereLocation = Hermes::Vec3(0.0f, 0.0f, 10.0f);

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

	/* OBJECT ROTATION DEBUG */
	float DebugObjectAngle = 0.0f;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
