#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"

namespace Hermes
{
	/**
	 * A generic class representing matrix
	 * NOTE : in Hermes, we assume row-major matrix memory layout.
	 * Because GLSL uses column-major layout, you should inverse all your matrix operations in GLSL
	 * to get proper results corresponding to the data you have on engine's side.
	 */
	template<int Rows, int Columns, typename InternalType>
	struct Matrix
	{
	public:
		Matrix(InternalType V = 0);

		template<int OtherRows, int OtherColumns, typename OtherInternalType>
		explicit Matrix(Matrix<OtherRows, OtherColumns, OtherInternalType> Other);

		bool operator==(const Matrix& Rhs) const;
		bool operator!=(const Matrix& Rhs) const;

		Matrix operator+(const Matrix& Rhs) const;
		Matrix operator-(const Matrix& Rhs) const;

		template<int OtherColumns>
		Matrix<Rows, OtherColumns, InternalType> operator*(const Matrix<Columns, OtherColumns, InternalType>& Rhs) const;

		void operator+=(const Matrix& Rhs);
		void operator-=(const Matrix& Rhs);

		template<int OutRows = Rows, int OutColumns = Columns>
		static std::enable_if_t<OutRows == OutColumns, Matrix> Identity();

		template<typename VectorInternalType>
		static Matrix<4, 4, InternalType> Translation(Vector3<VectorInternalType> Translation);

		template<typename AngleType>
		static Matrix<3, 3, InternalType> RotationX(AngleType Angle);
		template<typename AngleType>
		static Matrix<3, 3, InternalType> RotationY(AngleType Angle);
		template<typename AngleType>
		static Matrix<3, 3, InternalType> RotationZ(AngleType Angle);

		template<typename AngleVectorType>
		static Matrix<3, 3, InternalType> Rotation(Vector3<AngleVectorType> Rotation);

		template<typename ScaleVectorType>
		static Matrix<3, 3, InternalType> Scale(Vector3<ScaleVectorType> Scale);

		template<typename VectorType>
		static Matrix<4, 4, InternalType> LookAt(Vector3<VectorType> Origin, Vector3<VectorType> Forward, Vector3<VectorType> Up);

		/*
		 * Provides a perspective projection matrix
		 * Expects NDC range for Z axis to be [0;1]
		 * Expects near and far clip plane to be positive
		 * Expects camera to be looking into +Z direction(e.g. will clip any vertices which Z coordinate is not in range [near;far])
		 */
		template<typename T>
		static Matrix<4, 4, InternalType> Perspective(T VerticalFOV, T AspectRatio, T NearClipPlane, T FarClipPlane);

		struct RowProxy
		{
			InternalType& operator[](size_t Index);
		private:
			InternalType* Data;

			RowProxy(InternalType* InData) : Data(InData) {}

			friend struct Matrix;
		};

		struct ConstRowProxy
		{
			const InternalType& operator[](size_t Index) const;
		private:
			const InternalType* Data;

			ConstRowProxy(const InternalType* InData) : Data(InData) {}

			friend struct Matrix;
		};

		RowProxy operator[](size_t Index);
		ConstRowProxy operator[](size_t Index) const;

	private:
		InternalType Data[Rows * Columns];
	};

	using Mat3 = Matrix<3, 3, float>;
	using Mat4 = Matrix<4, 4, float>;

	template <int Rows, int Columns, typename InternalType>
	Matrix<Rows, Columns, InternalType>::Matrix(InternalType V)
	{
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Data[Index] = V;
		}
	}

	template <int Rows, int Columns, typename InternalType>
	template <int OtherRows, int OtherColumns, typename OtherInternalType>
	Matrix<Rows, Columns, InternalType>::Matrix(Matrix<OtherRows, OtherColumns, OtherInternalType> Other)
	{
		int TotalRows = Math::Min(Rows, OtherRows);
		int TotalColumns = Math::Min(Columns, OtherColumns);
		for (int Row = 0; Row < Rows; Row++)
		{
			for (int Column = 0; Column < Columns; Column++)
			{
				if (Row < TotalRows && Column < TotalColumns)
					this->operator[](Row)[Column] = InternalType(Other[Row][Column]);
				else
					this->operator[](Row)[Column] = 0;
			}
		}
	}

	template <int Rows, int Columns, typename InternalType>
	bool Matrix<Rows, Columns, InternalType>::operator==(const Matrix& Rhs) const
	{
		for (size_t Index = 0; Index < Rows * Columns; Index++)
			if (Data[Index] != Rhs.Data[Index])
				return false;
		return true;
	}

	template <int Rows, int Columns, typename InternalType>
	bool Matrix<Rows, Columns, InternalType>::operator!=(const Matrix& Rhs) const
	{
		return !operator==(Rhs);
	}

	template <int Rows, int Columns, typename InternalType>
	Matrix<Rows, Columns, InternalType> Matrix<Rows, Columns, InternalType>::operator+(const Matrix& Rhs) const
	{
		Matrix Res;
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Res.Data[Index] = Data[Index] + Rhs.Data[Index];
		}
		return Res;
	}

	template <int Rows, int Columns, typename InternalType>
	Matrix<Rows, Columns, InternalType> Matrix<Rows, Columns, InternalType>::operator-(const Matrix& Rhs) const
	{
		Matrix Res;
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Res.Data[Index] = Data[Index] - Rhs.Data[Index];
		}
		return Res;
	}

	template <int Rows, int Columns, typename InternalType>
	template <int OtherColumns>
	Matrix<Rows, OtherColumns, InternalType> Matrix<Rows, Columns, InternalType>::operator*(const Matrix<Columns, OtherColumns, InternalType>& Rhs) const
	{
		Matrix<Rows, OtherColumns, InternalType> Result;
		for (int Row = 0; Row < Rows; Row++)
		{
			for (int Column = 0; Column < OtherColumns; Column++)
			{
				InternalType Value = 0;
				for (int Index = 0; Index < Columns; Index++)
				{
					Value += this->operator[](Row)[Index] * Rhs[Index][Column];
				}
				Result[Row][Column] = Value;
			}
		}
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	void Matrix<Rows, Columns, InternalType>::operator+=(const Matrix& Rhs)
	{
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Data[Index] += Rhs.Data[Index];
		}
	}

	template <int Rows, int Columns, typename InternalType>
	void Matrix<Rows, Columns, InternalType>::operator-=(const Matrix& Rhs)
	{
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Data[Index] -= Rhs.Data[Index];
		}
	}

	template <int Rows, int Columns, typename InternalType>
	template <int OutRows, int OutColumns>
	std::enable_if_t<OutRows == OutColumns, Matrix<Rows, Columns, InternalType>> Matrix<Rows, Columns, InternalType>::Identity()
	{
		static Matrix<OutRows, OutColumns, InternalType> Result;
		if (Result[0][0] == 1)
			return Result;
		for (size_t Index = 0; Index < Rows; Index++)
			Result[Index][Index] = 1;
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename VectorInternalType>
	Matrix<4, 4, InternalType> Matrix<Rows, Columns, InternalType>::Translation(Vector3<VectorInternalType> Translation)
	{
		auto Result = Matrix<4, 4, InternalType>::Identity();
		Result[0][3] = Translation.X;
		Result[1][3] = Translation.Y;
		Result[2][3] = Translation.Z;
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename AngleType>
	Matrix<3, 3, InternalType> Matrix<Rows, Columns, InternalType>::RotationX(AngleType Angle)
	{
		Matrix<3, 3, InternalType> Result;
		Result[0][0] = 1;
		Result[1][1] = cos(Angle);
		Result[1][2] = -sin(Angle);
		Result[2][1] = sin(Angle);
		Result[2][2] = cos(Angle);
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename AngleType>
	Matrix<3, 3, InternalType> Matrix<Rows, Columns, InternalType>::RotationY(AngleType Angle)
	{
		Matrix<3, 3, InternalType> Result;
		Result[0][0] = cos(Angle);
		Result[0][3] = sin(Angle);
		Result[1][1] = 1;
		Result[2][0] = -sin(Angle);
		Result[2][2] = cos(Angle);
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename AngleType>
	Matrix<3, 3, InternalType> Matrix<Rows, Columns, InternalType>::RotationZ(AngleType Angle)
	{
		Matrix<3, 3, InternalType> Result;
		Result[0][0] = cos(Angle);
		Result[0][1] = -sin(Angle);
		Result[1][0] = sin(Angle);
		Result[1][1] = cos(Angle);
		Result[2][2] = 1;
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename AngleVectorType>
	Matrix<3, 3, InternalType> Matrix<Rows, Columns, InternalType>::Rotation(Vector3<AngleVectorType> Rotation)
	{
		return RotationY(Rotation.Y) * RotationX(Rotation.X) * RotationZ(Rotation.Z); // TODO : recheck order
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename ScaleVectorType>
	Matrix<3, 3, InternalType> Matrix<Rows, Columns, InternalType>::Scale(Vector3<ScaleVectorType> Scale)
	{
		Matrix<3, 3, InternalType> Result;
		Result[0][0] = static_cast<InternalType>(Scale.X);
		Result[1][1] = static_cast<InternalType>(Scale.Y);
		Result[2][2] = static_cast<InternalType>(Scale.Z);
		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename VectorType>
	Matrix<4, 4, InternalType> Matrix<Rows, Columns, InternalType>::LookAt(Vector3<VectorType> Origin, Vector3<VectorType> Forward, Vector3<VectorType> Up)
	{
		auto Result = Matrix<4, 4, InternalType>::Identity();
		Result[0][3] = -Origin.X;
		Result[1][3] = -Origin.Y;
		Result[2][3] = -Origin.Z;

		auto Right = Forward ^ Up;
		Result[0][0] = Right.X;
		Result[0][1] = Right.Y;
		Result[0][2] = Right.Z;

		Result[1][0] = Up.X;
		Result[1][1] = Up.Y;
		Result[1][2] = Up.Z;

		Result[2][0] = Forward.X;
		Result[2][1] = Forward.Y;
		Result[2][2] = Forward.Z;

		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename T>
	Matrix<4, 4, InternalType> Matrix<Rows, Columns, InternalType>::Perspective(T VerticalFOV, T AspectRatio, T NearClipPlane, T FarClipPlane)
	{
		Mat4 Result = { 0.0f };

		T CotanHalfFOV = Math::Cotan(VerticalFOV / static_cast<T>(2));

		Result[0][0] = CotanHalfFOV / AspectRatio;
		Result[1][1] = CotanHalfFOV;
		Result[2][2] = FarClipPlane / (FarClipPlane - NearClipPlane);
		Result[2][3] = -FarClipPlane * NearClipPlane / (FarClipPlane - NearClipPlane);
		Result[3][2] = static_cast<T>(1);

		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	InternalType& Matrix<Rows, Columns, InternalType>::RowProxy::operator[](size_t Index)
	{
		return Data[Index];
	}

	template <int Rows, int Columns, typename InternalType>
	const InternalType& Matrix<Rows, Columns, InternalType>::ConstRowProxy::operator[](size_t Index) const
	{
		return Data[Index];
	}

	template <int Rows, int Columns, typename InternalType>
	typename Matrix<Rows, Columns, InternalType>::RowProxy Matrix<Rows, Columns, InternalType>::operator[](size_t Index)
	{
		return RowProxy(&Data[Columns * Index]);
	}

	template <int Rows, int Columns, typename InternalType>
	typename Matrix<Rows, Columns, InternalType>::ConstRowProxy Matrix<Rows, Columns, InternalType>::operator[](size_t Index) const
	{
		return ConstRowProxy(&Data[Columns * Index]);
	}
}
