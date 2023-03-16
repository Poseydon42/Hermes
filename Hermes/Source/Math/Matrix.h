#pragma once

#include "Math/Vector.h"
#include "Math/Vector4.h"

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

		template<typename Enumerable>
		explicit Matrix(Enumerable InData);

		template<int OtherRows, int OtherColumns, typename OtherInternalType>
		explicit Matrix(Matrix<OtherRows, OtherColumns, OtherInternalType> Other);

		bool operator==(const Matrix& Rhs) const;
		bool operator!=(const Matrix& Rhs) const;

		Matrix operator+(const Matrix& Rhs) const;
		Matrix operator-(const Matrix& Rhs) const;

		template<int OtherColumns>
		Matrix<Rows, OtherColumns, InternalType> operator*(const Matrix<Columns, OtherColumns, InternalType>& Rhs) const;

		template<typename OtherType>
		Vector4<OtherType> operator*(const Vector4<OtherType>& Rhs) const;

		void operator+=(const Matrix& Rhs);
		void operator-=(const Matrix& Rhs);

		/*
		 * Computes an inverse matrix if possible.
		 *
		 * If the original matrix is singular the result is undefined.
		 */
		Matrix Inverse() const;

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
		 * Expects camera to be looking into -Z direction
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

	template<int Rows, int Columns, typename InternalType>
	template<typename Enumerable>
	Matrix<Rows, Columns, InternalType>::Matrix(Enumerable InData)
	{
		for (size_t Index = 0; Index < Rows * Columns; Index++)
		{
			Data[Index] = static_cast<InternalType>(InData[Index]);
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

	template<int Rows, int Columns, typename InternalType>
	template<typename OtherType>
	Vector4<OtherType> Matrix<Rows, Columns, InternalType>::operator*(const Vector4<OtherType>& Rhs) const
	{
		return Vec4{
			this->operator[](0)[0] * Rhs.X + this->operator[](0)[1] * Rhs.Y + this->operator[](0)[2] * Rhs.Z + this->operator[](0)[3] * Rhs.W,
			this->operator[](1)[0] * Rhs.X + this->operator[](1)[1] * Rhs.Y + this->operator[](1)[2] * Rhs.Z + this->operator[](1)[3] * Rhs.W,
			this->operator[](2)[0] * Rhs.X + this->operator[](2)[1] * Rhs.Y + this->operator[](2)[2] * Rhs.Z + this->operator[](2)[3] * Rhs.W,
			this->operator[](3)[0] * Rhs.X + this->operator[](3)[1] * Rhs.Y + this->operator[](3)[2] * Rhs.Z + this->operator[](3)[3] * Rhs.W,
		};
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

	template<int Rows, int Columns, typename InternalType>
	Matrix<Rows, Columns, InternalType> Matrix<Rows, Columns, InternalType>::Inverse() const
	{
		// Using Gauss-Jordan elimination
		static_assert(Rows == Columns, "Inverse of a non-square matrix is undefined.");

		// Create an augmented matrix that is twice the width of the original matrix and has the original matrix
		// on the left and the identity matrix on the right
		Matrix<Rows, 2 * Columns, InternalType> AugmentedMatrix;
		for (int Row = 0; Row < Rows; Row++)
		{
			for (int Column = 0; Column < Columns; Column++)
			{
				AugmentedMatrix[Row][Column] = this->operator[](Row)[Column];
			}
			AugmentedMatrix[Row][Columns + Row] = 1;
		}

		/*
		 * Loop over each row:
		 *  1. Divide the row by its first element to make its first element 1
		 *	2. Subtract the multiple of this row from all other rows to make their first N elements 0
		 */
		for (int Row = 0; Row < Rows; Row++)
		{
			// Dividing
			InternalType Divisor = AugmentedMatrix[Row][Row];
			for (int Column = 0; Column < 2 * Columns; Column++)
			{
				AugmentedMatrix[Row][Column] /= Divisor;
			}

			// Subtracting
			for (int RowToSubtractFrom = 0; RowToSubtractFrom < Rows; RowToSubtractFrom++)
			{
				if (RowToSubtractFrom == Row)
					continue;
				InternalType Multiple = AugmentedMatrix[RowToSubtractFrom][Row];
				// Starting with element [Row,Row] because the previous ones were became zero during previous iteration
				for (int Column = Row; Column < 2 * Columns; Column++)
				{
					AugmentedMatrix[RowToSubtractFrom][Column] -= Multiple * AugmentedMatrix[Row][Column];
				}
			}
		}

		// Retrieving the resulting matrix
		Matrix Result;
		for (int Row = 0; Row < Rows; Row++)
		{
			for (int Column = 0; Column < Columns; Column++)
			{
				Result[Row][Column] = AugmentedMatrix[Row][Column + Columns];
			}
		}

		return Result;
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
		Result[0][2] = sin(Angle);
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
		auto XAxis = (Up ^ Forward).SafeNormalize();
		auto YAxis = Up.SafeNormalize();
		auto ZAxis = -Forward.SafeNormalize();

		Result[0][3] = -(Origin | XAxis);
		Result[1][3] = -(Origin | YAxis);
		Result[2][3] = -(Origin | ZAxis);
		
		Result[0][0] = XAxis.X;
		Result[0][1] = XAxis.Y;
		Result[0][2] = XAxis.Z;

		Result[1][0] = YAxis.X;
		Result[1][1] = YAxis.Y;
		Result[1][2] = YAxis.Z;

		Result[2][0] = ZAxis.X;
		Result[2][1] = ZAxis.Y;
		Result[2][2] = ZAxis.Z;

		return Result;
	}

	template <int Rows, int Columns, typename InternalType>
	template <typename T>
	Matrix<4, 4, InternalType> Matrix<Rows, Columns, InternalType>::Perspective(T VerticalFOV, T AspectRatio, T NearClipPlane, T FarClipPlane)
	{
		Mat4 Result = { 0.0f };

		T CotanHalfFOV = Math::Cotan(VerticalFOV / static_cast<T>(2));

		Result[0][0] = CotanHalfFOV / AspectRatio;
		Result[1][1] = -CotanHalfFOV;
		Result[2][2] = NearClipPlane / (FarClipPlane - NearClipPlane);
		Result[2][3] = FarClipPlane * NearClipPlane / (FarClipPlane - NearClipPlane);
		Result[3][2] = static_cast<T>(-1);

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
