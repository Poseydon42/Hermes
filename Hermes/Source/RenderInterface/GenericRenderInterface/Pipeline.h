#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class DescriptorSetLayout;
		class Shader;
		
		struct VertexAttribute
		{
			uint32 Location;
			uint32 BindingIndex;
			uint32 Offset;
			DataFormat Format;
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
			std::vector<std::shared_ptr<Shader>> ShaderStages;
			std::vector<std::shared_ptr<DescriptorSetLayout>> DescriptorLayouts;
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
