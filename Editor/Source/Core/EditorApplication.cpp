#include "EditorApplication.h"

#include "ApplicationCore/GameLoop.h"
#include "Logging/Logger.h"
#include "Math/Frustum.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "UIEngine/Widgets/Containers/ViewportContainer.h"
#include "VirtualFilesystem/DirectoryFSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes::Editor
{
	// FIXME: replace with a proper camera
	class EditorCamera : public Camera
	{
	public:
		virtual Vec3 GetLocation() const override
		{
			return Location;
		}

		virtual Vec3 GetDirection() const override
		{
			return Direction;
		}

		virtual float GetVerticalFOV() const override
		{
			return Math::Pi / 4.0f;
		}

		virtual float GetNearZPlane() const override
		{
			return 0.1f;
		}

		virtual float GetFarZPlane() const override
		{
			return 1000.0f;
		}

		virtual Mat4 GetViewMatrix() const override
		{
			return Mat4::LookAt(Location, Direction, Vec3(0.0f, 1.0f, 0.0f));
		}

		virtual Mat4 GetProjectionMatrix(Vec2 ViewportDimensions) const override
		{
			return Mat4::Perspective(Math::Pi / 4.0f, ViewportDimensions.X / ViewportDimensions.Y, 0.1f, 1000.0f);
		}

		virtual Frustum GetFrustum(Vec2 ViewportDimensions) const override
		{
			Frustum Result = {};
			
			auto NearPlane = 0.1f;
			auto FarPlane = 1000.0f;
			auto VerticalFOV = Math::Pi / 4.0f;

			auto UpVector = Direction.Cross(RightVector).Normalized();
			auto VectorToCenterOfFarPlane = Direction * FarPlane;
			auto HalfVerticalSizeOfFarPlane = FarPlane * Math::Tan(0.5f * VerticalFOV);
			auto HalfHorizontalSizeOfFarPlane = HalfVerticalSizeOfFarPlane * ViewportDimensions.X / ViewportDimensions.Y;

			Result.Near = Plane(Direction, Location + Direction * NearPlane);
			Result.Far = Plane(-Direction, Location + VectorToCenterOfFarPlane);
			Result.Right = Plane((VectorToCenterOfFarPlane + RightVector * HalfHorizontalSizeOfFarPlane).Cross(UpVector), Location);
			Result.Left = Plane(UpVector.Cross(VectorToCenterOfFarPlane - RightVector * HalfHorizontalSizeOfFarPlane), Location);
			Result.Top = Plane(RightVector.Cross(VectorToCenterOfFarPlane + UpVector * HalfVerticalSizeOfFarPlane), Location);
			Result.Bottom = Plane((VectorToCenterOfFarPlane - UpVector * HalfVerticalSizeOfFarPlane).Cross(RightVector), Location);

			return Result;
		}

	private:
		static constexpr Vec3 Location = { 0.0f };
		static constexpr Vec3 Direction = { 0.0f, 0.0f, 1.0f };
		static constexpr Vec3 RightVector = { 1.0f, 0.0f, 0.0f };
	};

	bool EditorApplication::EarlyInit()
	{
		VirtualFilesystem::Mount("/Editor", MountMode::ReadOnly, 1, std::make_unique<DirectoryFSDevice>("Editor/Files"));
		VirtualFilesystem::Mount("/", MountMode::ReadWrite, 2, std::make_unique<DirectoryFSDevice>("Editor/Temp"));

		return true;
	}

	bool EditorApplication::Init()
	{
		HERMES_LOG_INFO("Initializing the editor");

		auto Viewport = UI::ViewportContainer::Create();
		GGameLoop->SetRootWidget(Viewport);

		Camera = std::make_shared<EditorCamera>();
		GGameLoop->OverrideCamera(Camera);

		return true;
	}

	void EditorApplication::Run(float)
	{
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
