#include "Transform.h"

namespace Hermes
{
	Mat4 Transform::GetTransformationMatrix() const
	{
		auto TranslationMatrix = Mat4::Translation(Translation);

		auto RotationMatrix = Mat4(Mat3::Rotation(Rotation));
		RotationMatrix[3][3] = 1.0f;

		auto ScaleMatrix = Mat4(Mat3::Scale(Scale));
		ScaleMatrix[3][3] = 1.0f;

		return TranslationMatrix * RotationMatrix * ScaleMatrix;
	}
}
