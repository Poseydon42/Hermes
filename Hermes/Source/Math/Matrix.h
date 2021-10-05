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

		bool operator==(const Matrix& Rhs) const;
		bool operator!=(const Matrix& Rhs) const;

		Matrix operator+(const Matrix& Rhs) const;
		Matrix operator-(const Matrix& Rhs) const;

		void operator+=(const Matrix& Rhs);
		void operator-=(const Matrix& Rhs);

		template<int OutRows = Rows, int OutColumns = Columns>
		static std::enable_if_t<OutRows == OutColumns, Matrix> Identity();
		static Matrix<4, 4, InternalType> Translation(Vector3<InternalType> Translation);

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
	Matrix<4, 4, InternalType> Matrix<Rows, Columns, InternalType>::Translation(Vector3<InternalType> Translation)
	{
		auto Result = Matrix<4, 4, InternalType>::Identity();
		Result[0][3] = Translation.X;
		Result[1][3] = Translation.Y;
		Result[2][3] = Translation.Z;
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
