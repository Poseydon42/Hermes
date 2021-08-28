#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class Shader;

		enum class VertexAttributeFormat
		{
			Undefined = 0,
			R4G4UnsignedNormalizedPack8,
			R4G4B4A4UnsignedNormalizedPack16,
			B4G4R4A4UnsignedNormalizedPack16,
			R5G6B5UnsignedNormalizedPack16,
			B5G6R5UnsignedNormalizedPack16,
			R5G5B5A1UnsignedNormalizedPack16,
			B5G5R5A1UnsignedNormalizedPack16,
			A1R5G5B5UnsignedNormalizedPack16,
			R8UnsignedNormalized,
			R8SignedNormalized,
			R8UnsignedScaled,
			R8SignedScaled,
			R8UnsignedInteger,
			R8SignedInteger,
			R8SRGB,
			R8G8UnsignedNormalized,
			R8G8SignedNormalized,
			R8G8UnsignedScaled,
			R8G8SignedScaled,
			R8G8UnsignedInteger,
			R8G8SignedInteger,
			R8G8SRGB,
			R8G8B8UnsignedNormalized,
			R8G8B8SignedNormalized,
			R8G8B8UnsignedScaled,
			R8G8B8SignedScaled,
			R8G8B8UnsignedInteger,
			R8G8B8SignedInteger,
			R8G8B8SRGB,
			B8G8R8UnsignedNormalized,
			B8G8R8SignedNormalized,
			B8G8R8UnsignedScaled,
			B8G8R8SignedScaled,
			B8G8R8UnsignedInteger,
			B8G8R8SignedInteger,
			B8G8R8SRGB,
			R8G8B8A8UnsignedNormalized,
			R8G8B8A8SignedNormalized,
			R8G8B8A8UnsignedScaled,
			R8G8B8A8SignedScaled,
			R8G8B8A8UnsignedInteger,
			R8G8B8A8SignedInteger,
			R8G8B8A8SRGB,
			B8G8R8A8UnsignedNormalized,
			B8G8R8A8SignedNormalized,
			B8G8R8A8UnsignedScaled,
			B8G8R8A8SignedScaled,
			B8G8R8A8UnsignedInteger,
			B8G8R8A8SignedInteger,
			B8G8R8A8SRGB,
			A8B8G8R8UnsignedNormalizedPack32,
			A8B8G8R8SignedNormalizedPack32,
			A8B8G8R8UnsignedScaledPack32,
			A8B8G8R8SignedScaledPack32,
			A8B8G8R8UnsignedIntegerPack32,
			A8B8G8R8SignedIntegerPack32,
			A8B8G8R8SRGBPack32,
			A2R10G10B10UnsignedNormalizedPack32,
			A2R10G10B10SignedNormalizedPack32,
			A2R10G10B10UnsignedScaledPack32,
			A2R10G10B10SignedScaledPack32,
			A2R10G10B10UnsignedIntegerPack32,
			A2R10G10B10SignedIntegerPack32,
			A2B10G10R10UnsignedNormalizedPack32,
			A2B10G10R10SignedNormalizedPack32,
			A2B10G10R10UnsignedScaledPack32,
			A2B10G10R10SignedScaledPack32,
			A2B10G10R10UnsignedIntegerPack32,
			A2B10G10R10SignedIntegerPack32,
			R16UnsignedNormalized,
			R16SignedNormalized,
			R16UnsignedScaled,
			R16SignedScaled,
			R16UnsignedInteger,
			R16SignedInteger,
			R16SignedFloat,
			R16G16UnsignedNormalized,
			R16G16SignedNormalized,
			R16G16UnsignedScaled,
			R16G16SignedScaled,
			R16G16UnsignedInteger,
			R16G16SignedInteger,
			R16G16SignedFloat,
			R16G16B16UnsignedNormalized,
			R16G16B16SignedNormalized,
			R16G16B16UnsignedScaled,
			R16G16B16SignedScaled,
			R16G16B16UnsignedInteger,
			R16G16B16SignedInteger,
			R16G16B16SignedFloat,
			R16G16B16A16UnsignedNormalized,
			R16G16B16A16SignedNormalized,
			R16G16B16A16UnsignedScaled,
			R16G16B16A16SignedScaled,
			R16G16B16A16UnsignedInteger,
			R16G16B16A16SignedInteger,
			R16G16B16A16SignedFloat,
			R32UnsignedInteger,
			R32SignedInteger,
			R32SignedFloat,
			R32G32UnsignedInteger,
			R32G32SignedInteger,
			R32G32SignedFloat,
			R32G32B32UnsignedInteger,
			R32G32B32SignedInteger,
			R32G32B32SignedFloat,
			R32G32B32A32UnsignedInteger,
			R32G32B32A32SignedInteger,
			R32G32B32A32SignedFloat,
			R64UnsignedInteger,
			R64SignedInteger,
			R64SignedFloat,
			R64G64UnsignedInteger,
			R64G64SignedInteger,
			R64G64SignedFloat,
			R64G64B64UnsignedInteger,
			R64G64B64SignedInteger,
			R64G64B64SignedFloat,
			R64G64B64A64UnsignedInteger,
			R64G64B64A64SignedInteger,
			R64G64B64A64SignedFloat,
			B10G11R11UnsignedFloatPack32,
			E5B9G9R9UnsignedFloatPack32,
			D16UnsignedNormalized,
			X8D24UnsignedNormalizedPack32,
			D32SignedFloat,
			S8UnsignedInteger,
			D16UnsignedNormalizedS8UnsignedInteger,
			D24UnsignedNormalizedS8UnsignedInteger,
			D32SignedFloatS8UnsignedInteger
		};
		
		struct VertexAttribute
		{
			uint32 Location;
			uint32 BindingIndex;
			uint32 Offset;
			VertexAttributeFormat Format;
		};

		struct VertexBinding
		{
			uint32 Index;
			uint32 Stride;
			bool IsPerInstance;
		};
		
		struct VertexInputDescription
		{
			std::vector<VertexAttribute> VertexAttributes;
			std::vector<VertexBinding> VertexBindings;
		};

		enum class TopologyType
		{
			PointList,
			LineList,
			LineStrip,
			TriangleList,
			TriangleStrip,
			TriangleFan
			// TODO : more of them maybe?
		};

		struct InputAssemblerDescription
		{
			TopologyType Topology;
		};

		struct ViewportDescription
		{
			Vec2ui Origin;
			Vec2ui Dimensions;
		};
		
		enum class FillMode
		{
			Fill,
			Lines,
			Points
		};

		enum class CullMode
		{
			Front,
			Back,
			Both
		};

		enum class FaceDirection
		{
			Clockwise,
			CounterClockwise
		};
		
		struct RasterizerDescription
		{
			FillMode Fill;
			CullMode Cull;
			FaceDirection Direction;
			// TODO : depth bias?
		};

		enum class ComparisonOperator
		{
			AlwaysFail,
			NotEqual,
			Less,
			LessOrEqual,
			Equal,
			GreaterOrEqual,
			Greater,
			AlwaysSucceed
		};
		
		struct DepthStencilDescription
		{
			bool IsDepthTestEnabled;
			bool IsDepthWriteEnabled;
			ComparisonOperator ComparisonMode;
			// TODO : stencil operators :^)
		};

		struct ColorBlendingDescription
		{
			// TODO : per-framebuffer blending settings
			// TODO : logic operators
			// TODO : proper implementation. Currently this is just stub, as we don't need to use any blending to draw simple triangle. But we will eventually :^)
		};

		struct PipelineDescription
		{
			// TODO : all these things should be separate for each subpass
			std::vector<std::shared_ptr<Shader>> ShaderStages;
			VertexInputDescription VertexInput;
			InputAssemblerDescription InputAssembler;
			// TODO : tessellation stage
			ViewportDescription Viewport; // TODO : scissors
			RasterizerDescription Rasterizer;
			// TODO : multisample
			DepthStencilDescription DepthStencilStage;
			ColorBlendingDescription ColorBlending;
			// TODO : dynamic pipeline stages :^)
		};

		class HERMES_API Pipeline
		{
			ADD_DEFAULT_CONSTRUCTOR(Pipeline)
			ADD_DEFAULT_DESTRUCTOR(Pipeline)
			MAKE_NON_COPYABLE(Pipeline)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Pipeline)
		};
	}
}
