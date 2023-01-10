#pragma once

#include "InputFileReader.h"

namespace Hermes::Tools
{
	class OBJReader : public IInputFileReader
	{
	public:
		virtual bool Read(StringView Path) override;

		virtual bool HasTangents() const override;

		virtual bool IsTriangulated() const override;

		virtual std::span<const Vertex> GetVertices() const override;

		virtual std::span<const uint32> GetIndices() const override;

	private:
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		bool HasNonTriangleFaces = false;
	};
}