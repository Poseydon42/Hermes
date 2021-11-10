#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Core/Application/GameLoop.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/MeshAsset.h"
#include "Core/Application/InputEngine.h"
#include "Math/Vector.h"

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		auto SphereMesh = Hermes::Asset::As<Hermes::MeshAsset>(Hermes::AssetLoader::Load(L"sphere"));
		auto SphereMeshBuffer = Hermes::MeshBuffer::CreateFromAsset(SphereMesh);
		auto AlbedoTextureAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"pbr_test_albedo"));

		auto AlbedoTexture = Hermes::Texture::CreateFromAsset(AlbedoTextureAsset);
		std::shared_ptr<Hermes::Material> PBRMaterial = std::make_shared<Hermes::Material>(std::vector{ AlbedoTexture });
		Hermes::MeshProxy SphereMeshProxy =
		{
			Hermes::Mat4::Translation(Hermes::Vec3{ 0.0f, 0.0f, 10.0f }),
			*SphereMeshBuffer,
			PBRMaterial
		};
		Hermes::GGameLoop->GetScene().AddMesh(SphereMeshProxy);

		Hermes::PointLightProxy PointLight = {};
		PointLight.Color = { 1.0f, 1.0f, 0.9f, 0.0f };
		PointLight.Position = { 0.0f, 3.0f, 0.0f, 0.0f };
		Hermes::GGameLoop->GetScene().AddPointLight(PointLight);

		return true;
	}

	void Run(float DeltaTime) override
	{
		const auto& InputEngine = Hermes::GGameLoop->GetInputEngine();

		static constexpr float CameraRotationSpeed = 150.0f;
		Hermes::Vec2 MouseDelta = InputEngine.GetDeltaMousePosition();
		MouseDelta = MouseDelta.SafeNormalize() * DeltaTime * CameraRotationSpeed * Hermes::Vec2 { 1.0f, -1.0f };
		CameraPitch = Hermes::Math::Clamp(-85.0f, 85.0f, CameraPitch + MouseDelta.Y);
		CameraYaw += MouseDelta.X;
		CameraYaw = fmod(CameraYaw, 360.0f);
		if (CameraYaw > 180.0f)
			CameraYaw = 360.0f - CameraYaw;
		if (CameraYaw < -180.0f)
			CameraYaw = 360.0f + CameraYaw;
		Hermes::Vec3 CameraForward;
		CameraForward.X = -Hermes::Math::Sin(Hermes::Math::Radians(CameraYaw));
		CameraForward.Y = Hermes::Math::Sin(Hermes::Math::Radians(CameraPitch));
		CameraForward.Z = Hermes::Math::Cos(Hermes::Math::Radians(CameraYaw)) * Hermes::Math::Cos(Hermes::Math::Radians(CameraPitch));
		CameraForward.Normalize();
		Hermes::Vec3 GlobalUp = { 0.0f, 1.0f, 0.0f };
		Hermes::Vec3 CameraRight = (GlobalUp ^ CameraForward).Normalize();
		
		Hermes::Vec3 DeltaCameraPosition = {};
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::W))
			DeltaCameraPosition += CameraForward;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::S))
			DeltaCameraPosition -= CameraForward;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::D))
			DeltaCameraPosition += -CameraRight;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::A))
			DeltaCameraPosition -= -CameraRight;
		DeltaCameraPosition.SafeNormalize();
		DeltaCameraPosition *= DeltaTime * 10.0f;
		CameraPos += DeltaCameraPosition;

		Hermes::GGameLoop->GetScene().UpdateCameraTransform(CameraPos, CameraPitch, CameraYaw);
	}

	void Shutdown() override
	{
	}

private:
	Hermes::Vec3 CameraPos = {0.0f, 0.0f, 0.0f};
	float CameraPitch = 0.0f, CameraYaw = 0.0f;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
