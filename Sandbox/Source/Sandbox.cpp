#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Profiling.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GameLoop.h"
#include "ApplicationCore/InputEngine.h"
#include "Core/Event/Event.h"
#include "Math/Vector.h"
#include "RenderingEngine/Scene/FPSCamera.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Renderer.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "Vulkan/Swapchain.h"
#include "World/Components/DirectionalLightComponent.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/PointLightComponent.h"
#include "World/Components/TransformComponent.h"

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto& AssetCache = Hermes::GGameLoop->GetAssetCache();
		auto SphereAssetHandle = AssetCache.Create("/sphere");

		auto MaybeTexturedMaterialInstance = Hermes::MaterialInstance::CreateFromJSON(Hermes::VirtualFilesystem::ReadFileAsString("/pbr_test.hmat").value_or(""));
		HERMES_ASSERT(MaybeTexturedMaterialInstance);
		TexturedMaterialInstance = std::move(MaybeTexturedMaterialInstance.value());

		auto& World = Hermes::GGameLoop->GetWorld();

		static constexpr int SphereCountInOneDirection = 19;
		static constexpr float SphereSpacing = 15.0f;
		for (int X = -SphereCountInOneDirection / 2; X <= SphereCountInOneDirection / 2; X++)
		{
			for (int Y = -SphereCountInOneDirection / 2; Y <= SphereCountInOneDirection / 2; Y++)
			{
				auto SphereEntity = World.CreateEntity();

				auto& Transform = World.AddComponent<Hermes::TransformComponent>(SphereEntity);
				Transform.Transform.Translation = { static_cast<float>(X) * SphereSpacing, 0, static_cast<float>(Y) * SphereSpacing };

				auto& SphereMeshComponent = World.AddComponent<Hermes::MeshComponent>(SphereEntity);
				SphereMeshComponent.Mesh = SphereAssetHandle;
				SphereMeshComponent.Material = TexturedMaterialInstance.get();

				auto PointLightEntity = World.CreateEntity();
				auto& LightTransform = World.AddComponent<Hermes::TransformComponent>(PointLightEntity);
				LightTransform.Transform = Transform.Transform;
				LightTransform.Transform.Translation.Y = 10.0f;

				auto& PointLightComponent = World.AddComponent<Hermes::PointLightComponent>(PointLightEntity);
				PointLightComponent.Color = { 0.0f, 0.3f, 1.0f };
				PointLightComponent.Intensity = 50.0f;
			}
		}

		auto DirectionalLightEntity = World.CreateEntity();
		auto& DirectionalLight = World.AddComponent<Hermes::DirectionalLightComponent>(DirectionalLightEntity);
		DirectionalLight.Direction = { -1.0f, -1.0f, -1.0f };
		DirectionalLight.Direction.Normalize();
		DirectionalLight.Color = { 1.0f, 1.0f, 1.0f };
		DirectionalLight.Intensity = 10.0f;

		Camera = std::make_unique<Hermes::FPSCamera>(Hermes::Vec3(0.0f, 3.0f, 0.0f), 0.0f, 0.0f, 0.5f, 25.0f, 50.0f,
		                                             Hermes::Vec2(Hermes::Renderer::Get().GetSwapchain().GetDimensions()), true);
		Hermes::GGameLoop->SetCamera(Camera);

		Hermes::GGameLoop->GetInputEngine().GetEventQueue().Subscribe<SandboxApp, &SandboxApp::KeyEventHandler>(Hermes::KeyEvent::GetStaticType(), this);

		return true;
	}

	void Run(float DeltaTime) override
	{
		HERMES_PROFILE_FUNC();
		const auto& InputEngine = Hermes::GGameLoop->GetInputEngine();
		
		Hermes::Vec2 CameraMovementInput = {};
#if 1
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::W))
			CameraMovementInput.X += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::S))
			CameraMovementInput.X -= 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::D))
			CameraMovementInput.Y += 1.0f;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::A))
			CameraMovementInput.Y -= 1.0f;
#endif
		
		Camera->ApplyMovementInput(CameraMovementInput, DeltaTime);
#if 0
		Camera->ApplyRotationInput({}, DeltaTime);
#else
		Camera->ApplyRotationInput(InputEngine.GetDeltaMousePosition(), DeltaTime);
#endif

		if (AnisotropyChanged)
		{
			Hermes::GraphicsSettings Settings;
			Settings.AnisotropyLevel = AnisotropyEnabled ? 16.0f : 0.0f;
			Hermes::Renderer::Get().UpdateGraphicsSettings(Settings);
		}
	}

	void Shutdown() override
	{
	}

private:
	bool AnisotropyEnabled = false, AnisotropyChanged = false;
	std::shared_ptr<Hermes::FPSCamera> Camera;
	std::shared_ptr<Hermes::MaterialInstance> TexturedMaterialInstance;

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
