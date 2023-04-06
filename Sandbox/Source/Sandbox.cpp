#include "UIEngine/Widgets/Containers/VerticalContainerWidget.h"
#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Profiling.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GameLoop.h"
#include "ApplicationCore/InputEngine.h"
#include "Core/Event/Event.h"
#include "Math/Frustum.h"
#include "Math/Vector.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "UIEngine/Widgets/PanelWidget.h"
#include "UIEngine/Window.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "World/Components/DirectionalLightComponent.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/PointLightComponent.h"
#include "World/Components/TagComponent.h"
#include "World/Components/TransformComponent.h"

class SimpleCamera : public Hermes::Camera
{
public:
	SimpleCamera(Hermes::Vec3 InLocation, Hermes::Vec3 InDirection, Hermes::Vec3 InRightVector, float InVerticalFOV, float InNearPlane, float InFarPlane, Hermes::Vec2 InViewportDimensions)
		: Location(InLocation)
		, Direction(InDirection)
		, RightVector(InRightVector)
		, VerticalFOV(InVerticalFOV)
		, NearPlane(InNearPlane)
		, FarPlane(InFarPlane)
		, ViewportDimensions(InViewportDimensions)
	{
	}

	virtual Hermes::Vec3 GetLocation() const override
	{
		return Location;
	}

	virtual Hermes::Vec3 GetDirection() const override
	{
		return Direction;
	}

	virtual float GetVerticalFOV() const override
	{
		return VerticalFOV;
	}

	virtual float GetNearZPlane() const override
	{
		return NearPlane;
	}

	virtual float GetFarZPlane() const override
	{
		return FarPlane;
	}

	virtual void UpdateViewportDimensions(Hermes::Vec2 NewDimensions) override
	{
		ViewportDimensions = NewDimensions;
	}

	virtual Hermes::Mat4 GetViewMatrix() const override
	{
		auto UpVector = (Direction ^ RightVector).Normalize();
		return Hermes::Mat4::LookAt(Location, Direction, UpVector);
	}

	virtual Hermes::Mat4 GetProjectionMatrix() const override
	{
		return Hermes::Mat4::Perspective(Hermes::Math::Radians(VerticalFOV), ViewportDimensions.X / ViewportDimensions.Y, NearPlane, FarPlane);
	}

	virtual Hermes::Frustum GetFrustum() const override
	{
		Hermes::Frustum Result = {};

		auto UpVector = (Direction ^ RightVector).Normalize();
		auto VectorToCenterOfFarPlane = Direction * FarPlane;
		auto HalfVerticalSizeOfFarPlane = FarPlane * Hermes::Math::Tan(0.5f * Hermes::Math::Radians(VerticalFOV));
		auto HalfHorizontalSizeOfFarPlane = HalfVerticalSizeOfFarPlane * ViewportDimensions.X / ViewportDimensions.Y;

		Result.Near = Hermes::Plane(Direction, Location + Direction * NearPlane);
		Result.Far = Hermes::Plane(-Direction, Location + VectorToCenterOfFarPlane);
		Result.Right = Hermes::Plane((VectorToCenterOfFarPlane + RightVector * HalfHorizontalSizeOfFarPlane) ^ UpVector,
		                             Location);
		Result.Left = Hermes::Plane(UpVector ^ (VectorToCenterOfFarPlane - RightVector * HalfHorizontalSizeOfFarPlane),
		                            Location);
		Result.Top = Hermes::Plane(RightVector ^ (VectorToCenterOfFarPlane + UpVector * HalfVerticalSizeOfFarPlane),
		                           Location);
		Result.Bottom = Hermes::Plane((VectorToCenterOfFarPlane - UpVector * HalfVerticalSizeOfFarPlane) ^ RightVector,
		                              Location);

		return Result;
	}

private:
	Hermes::Vec3 Location, Direction, RightVector;
	float VerticalFOV = 0.0f, NearPlane = 0.0f, FarPlane = 0.0f;
	Hermes::Vec2 ViewportDimensions;
};

class CameraSystem : public Hermes::ISystem
{
public:
	static constexpr float MovementSpeed = 2.0f;
	static constexpr float RotationSpeed = 40.0f;

	virtual void Run(Hermes::World& World, Hermes::Scene& Scene, float DeltaTime) const override
	{
		auto PotentialCameras = World.View<Hermes::TagComponent, Hermes::TransformComponent>();
		for (auto Camera : PotentialCameras)
		{
			if (World.GetComponent<Hermes::TagComponent>(Camera)->Tag != "Camera")
				continue;
			auto* Transform = World.GetComponent<Hermes::TransformComponent>(Camera);
			auto& Pitch = Transform->Transform.Rotation.X;
			auto& Yaw = Transform->Transform.Rotation.Z;
			const auto& InputEngine = Hermes::GGameLoop->GetInputEngine();

			auto MouseRotationInput = InputEngine.GetDeltaMousePosition();

			MouseRotationInput *= DeltaTime;
			float DeltaPitch = -1.0f * MouseRotationInput.Y * RotationSpeed;
			float DeltaYaw = MouseRotationInput.X * RotationSpeed;

			Pitch = Hermes::Math::Clamp(Hermes::Math::Radians(-85.0f), Hermes::Math::Radians(85.0f), Pitch + DeltaPitch);
			Yaw += DeltaYaw;
			Yaw = fmod(Yaw, 2 * Hermes::Math::Pi);
			if (Yaw > Hermes::Math::Pi)
				Yaw = -(2 * Hermes::Math::Pi - Yaw);
			if (Yaw < -Hermes::Math::Pi)
				Yaw = 2 * Hermes::Math::Pi + Yaw;

			Hermes::Vec3 Direction = {};
			Direction.X = Hermes::Math::Sin(Yaw) * Hermes::Math::Cos(Pitch);
			Direction.Y = Hermes::Math::Sin(Pitch);
			Direction.Z = Hermes::Math::Cos(Yaw) * Hermes::Math::Cos(Pitch);
			Direction.Normalize();

			Hermes::Vec3 GlobalUp = { 0.0f, 1.0f, 0.0f };
			auto RightVector = (GlobalUp ^ Direction).Normalize();

			Hermes::Vec2 CameraMovementInput = {};
			if (InputEngine.IsKeyPressed(Hermes::KeyCode::W))
				CameraMovementInput.X += 1.0f;
			if (InputEngine.IsKeyPressed(Hermes::KeyCode::S))
				CameraMovementInput.X -= 1.0f;
			if (InputEngine.IsKeyPressed(Hermes::KeyCode::D))
				CameraMovementInput.Y += 1.0f;
			if (InputEngine.IsKeyPressed(Hermes::KeyCode::A))
				CameraMovementInput.Y -= 1.0f;
			CameraMovementInput *= DeltaTime;

			auto DeltaLocation = Direction * CameraMovementInput.X + RightVector * CameraMovementInput.Y;
			DeltaLocation = DeltaLocation.SafeNormalize() * MovementSpeed;

			Transform->Transform.Translation += DeltaLocation;

			Scene.ChangeActiveCamera(std::make_shared<SimpleCamera>(Transform->Transform.Translation, Direction, RightVector, 50.0f, 0.1f, 1000.0f, Hermes::Vec2{ 1280.0f, 720.0f }));

			break;
		}
	}
};

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto& AssetCache = Hermes::GGameLoop->GetAssetCache();
		auto SphereAssetHandle = AssetCache.Create("/sphere");

		SolidColorMaterialInstanceHandle = AssetCache.Create("/mi_metal");
		HERMES_ASSERT(SolidColorMaterialInstanceHandle != Hermes::GInvalidAssetHandle);

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
				SphereMeshComponent.MaterialInstance = SolidColorMaterialInstanceHandle;

				auto PointLightEntity = World.CreateEntity();
				auto& LightTransform = World.AddComponent<Hermes::TransformComponent>(PointLightEntity);
				LightTransform.Transform = Transform.Transform;
				LightTransform.Transform.Translation.Y = 10.0f;

				auto& PointLightComponent = World.AddComponent<Hermes::PointLightComponent>(PointLightEntity);
				PointLightComponent.Color = { 1.0f, 1.0f, 1.0f };
				PointLightComponent.Intensity = 50.0f;
			}
		}

		auto DirectionalLightEntity = World.CreateEntity();
		auto& DirectionalLight = World.AddComponent<Hermes::DirectionalLightComponent>(DirectionalLightEntity);
		DirectionalLight.Direction = { -1.0f, -1.0f, -1.0f };
		DirectionalLight.Direction.Normalize();
		DirectionalLight.Color = { 1.0f, 1.0f, 1.0f };
		DirectionalLight.Intensity = 10.0f;

		auto CameraEntity = World.CreateEntity();
		World.AddComponent<Hermes::TransformComponent>(CameraEntity);
		World.AddComponent<Hermes::TagComponent>(CameraEntity).Tag = "Camera";

		World.AddSystem(std::make_unique<CameraSystem>());

		Hermes::GGameLoop->GetInputEngine().GetEventQueue().Subscribe(Hermes::KeyEvent::GetStaticType(), [this](const Hermes::IEvent& Event) { KeyEventHandler(Event); });

		auto VerticalContainer = Hermes::UI::VerticalContainerWidget::Create(nullptr);
		VerticalContainer->AddChild(Hermes::UI::PanelWidget::Create(VerticalContainer, { 100, 20 }, { 1.0f, 0.0f, 0.0f }));
		VerticalContainer->AddChild(Hermes::UI::PanelWidget::Create(VerticalContainer, { 30, 100 }, { 0.0f, 1.0f, 0.0f }));
		UIWindow = std::make_unique<Hermes::UI::Window>(std::move(VerticalContainer), Hermes::Vec2ui{ 100, 300 });

		return true;
	}

	void Run(float) override
	{
		HERMES_PROFILE_FUNC();

		if (AnisotropyChanged)
		{
			Hermes::GraphicsSettings Settings;
			Settings.AnisotropyLevel = AnisotropyEnabled ? 16.0f : 0.0f;
			Hermes::Renderer::Get().UpdateGraphicsSettings(Settings);
		}

		Hermes::Renderer::Get().AddWindow(*UIWindow, { 0, 0 });
	}

	void Shutdown() override
	{
	}

private:
	bool AnisotropyEnabled = false, AnisotropyChanged = false;
	Hermes::AssetHandle SolidColorMaterialInstanceHandle = Hermes::GInvalidAssetHandle;
	std::unique_ptr<Hermes::UI::Window> UIWindow;

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
