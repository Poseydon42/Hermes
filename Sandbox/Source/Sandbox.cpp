﻿#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Profiling.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/MeshAsset.h"
#include "ApplicationCore/InputEngine.h"
#include "Core/Event/Event.h"
#include "Math/Vector.h"
#include "RenderingEngine/Scene/FPSCamera.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Texture.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "Vulkan/Swapchain.h"

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto MaybeSphereMeshAsset = Hermes::GGameLoop->GetAssetCache().Get<Hermes::MeshAsset>("/sphere");
		if (!MaybeSphereMeshAsset.has_value() || MaybeSphereMeshAsset.value() == nullptr)
			return false;
		auto& SphereMesh = Hermes::Asset::As<Hermes::MeshAsset>(*MaybeSphereMeshAsset.value());
		auto BoundingVolume = SphereMesh.GetBoundingVolume();
		auto SphereMeshBuffer = Hermes::MeshBuffer::CreateFromAsset(SphereMesh);
		
		auto MaybeTexturedMaterialInstance = Hermes::MaterialInstance::CreateFromJSON(Hermes::VirtualFilesystem::ReadFileAsString("/pbr_test.hmat").value_or(""));
		HERMES_ASSERT(MaybeTexturedMaterialInstance);
		TexturedMaterialInstance = std::move(MaybeTexturedMaterialInstance.value());
		
		auto MaybeSolidColorMaterialInstance = Hermes::MaterialInstance::CreateFromJSON(Hermes::VirtualFilesystem::ReadFileAsString("/test_solid_color.hmat").value_or(""));
		HERMES_ASSERT(MaybeSolidColorMaterialInstance);
		SolidColorMaterialInstance = std::move(MaybeSolidColorMaterialInstance.value());

		static constexpr Hermes::int32 SphereCountInSingleDimension = 39;
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
					TexturedMaterialInstance
				};
				Hermes::GGameLoop->GetScene().AddMesh(SphereMeshProxy);
			}
		}

		static constexpr Hermes::int32 PointLightCountInSingleDimension = 15;
		static constexpr float GridDistanceBetweenPointLights = 25.0f;
		static constexpr float LightHeight = 4.0f;
		static constexpr float LightPower = 200.0f;
		for (auto LightX = -PointLightCountInSingleDimension / 2;
		     LightX <= PointLightCountInSingleDimension / 2;
		     LightX++)
		{
			for (auto LightZ = -PointLightCountInSingleDimension / 2;
			     LightZ <= PointLightCountInSingleDimension / 2;
			     LightZ++)
			{
				Hermes::PointLightProxy PointLight = {};
				PointLight.Color = { 1.0f, 1.0f, 0.9f, LightPower };
				PointLight.Position = {
					static_cast<float>(LightX) * GridDistanceBetweenPointLights,
					LightHeight,
					static_cast<float>(LightZ) * GridDistanceBetweenPointLights,
					0.0f
				};
				Hermes::GGameLoop->GetScene().AddPointLight(PointLight);
			}
		}

		Hermes::DirectionalLightProxy DirectionalLight = {};
		DirectionalLight.Direction = Hermes::Vec3(0.0f, -1.0f, -1.0f).Normalize();
		DirectionalLight.Color = { 1.0f, 1.0f, 1.0f };
		DirectionalLight.Intensity = 8.0f;
		Hermes::GGameLoop->GetScene().AddDirectionalLight(DirectionalLight);

		Camera = std::make_unique<Hermes::FPSCamera>(Hermes::Vec3(0.0f, 3.0f, 0.0f), 0.0f, 0.0f, 0.5f, 25.0f, 50.0f,
		                                             Hermes::Vec2(Hermes::Renderer::Get().GetSwapchain().GetDimensions()), true);
		Hermes::GGameLoop->GetScene().ChangeActiveCamera(Camera);

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
	std::shared_ptr<Hermes::MaterialInstance> TexturedMaterialInstance, SolidColorMaterialInstance;

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

			if (KeyEvent.GetKeyCode() == Hermes::KeyCode::E)
			{
				std::shared_ptr<Hermes::MaterialInstance> NewMaterialInstance = nullptr;
				if (KeyEvent.IsPressEvent())
					NewMaterialInstance = SolidColorMaterialInstance;
				else
					NewMaterialInstance = TexturedMaterialInstance;

				auto& Scene = Hermes::GGameLoop->GetScene();
				for (const auto& Mesh : Scene.GetMeshes())
				{
					const_cast<Hermes::MeshProxy&>(Mesh).Material = NewMaterialInstance;
				}
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
