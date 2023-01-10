#pragma once

#include <memory>

#include "InputFileReader.h"

namespace Hermes::Tools
{
	class FileProcessor
	{
	public:
		FileProcessor(String InFileName, bool InFlipVertexOrder);

		bool Run() const;

	private:
		std::unique_ptr<IInputFileReader> InputFileReader;

		String InputFileName;
		String OutputFileName;

		bool ShouldFlipVertexOrder = false;

		void FlipVertexOrder(std::vector<uint32>& Indices) const;

		/*
		 * Computes tangents for each vertex in the given array and stores them. Expects the mesh to be triangulated
		 */
		void ComputeTangents(std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices) const;
	};
}
