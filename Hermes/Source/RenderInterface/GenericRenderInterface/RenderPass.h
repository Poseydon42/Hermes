#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/EnumClassOperators.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

#include <vector>

namespace Hermes
{
	namespace RenderInterface
	{
		enum class AttachmentLoadOp
		{
			Load, // Attachment content is preserved
			Clear, // Attachment content is cleared
			Undefined // Don't care about attachment content, its state is undefined
		};

		enum class AttachmentStoreOp
		{
			Store, // Attachment content is stored
			Undefined // Don't care about attachment content, its state is undefined
		};

		struct RenderPassAttachment
		{
			AttachmentLoadOp LoadOp;
			AttachmentStoreOp StoreOp;
			AttachmentLoadOp StencilLoadOp;
			AttachmentStoreOp StencilStoreOp;
			ImageFormat Format;
			ImageLayout LayoutBeforeBegin;
			ImageLayout LayoutAtEnd;
			// TODO : sample count, flags
		};
		
		struct SubpassAttachmentReference
		{
			uint32 Index; // Attachment index in RenderPassDescription::Attachments vector
			ImageLayout Layout;
		};

		struct SubpassDescription
		{
			// TODO : pipeline bind point - add it when we're gonna have compute operations supported :)
			std::vector<SubpassAttachmentReference> InputAttachments;
			std::vector<SubpassAttachmentReference> ColorAttachments;
			bool IsDepthStencilAttachmentUsed;
			SubpassAttachmentReference DepthStencilAttachment;
			std::vector<uint32> PreservedAttachmentsIndices; // Indices of attachments that aren't used by this subpass, but their content should be preserved during subpass for later usage
		};

		// TODO : move in some more appropriate place when we would have one :)
		enum class PipelineStage
		{
			TopOfPipe = 0x0001,
			DrawIndirect = 0x0002,
			VertexInput = 0x0004,
			VertexShader = 0x0008,
			TessellationControlShader = 0x0010,
			TessellationEvaluationShader = 0x0020,
			GeometryShader = 0x0040,
			FragmentShader = 0x0080,
			EarlyFragmentTests = 0x0100,
			LateFragmentTests = 0x0200,
			ColorAttachmentOutput = 0x0400,
			Transfer = 0x800,
			BottomOfPipe = 0x1000,
			Host = 0x2000,
			AllGraphics = DrawIndirect | VertexInput | VertexShader | TessellationControlShader | GeometryShader | FragmentShader | EarlyFragmentTests | LateFragmentTests | ColorAttachmentOutput,
			AllCommands = 0x8000
		};

		ENUM_CLASS_OPERATORS(PipelineStage)

		enum class AccessType
		{
			IndirectCommandRead = 0x00000001,
			IndexRead = 0x00000002,
			VertexAttributeRead = 0x00000004,
			UniformRead = 0x00000008,
			InputAttachmentRead = 0x00000010,
			ShaderRead = 0x00000020,
			ShaderWrite = 0x00000040,
			ColorAttachmentRead = 0x00000080,
			ColorAttachmentWrite = 0x00000100,
			DepthStencilAttachmentRead = 0x00000200,
			DepthStencilAttachmentWrite = 0x00000400,
			TransferRead = 0x00000800,
			TransferWrite = 0x00001000,
			HostRead = 0x00002000,
			HostWrite = 0x00004000,
			MemoryRead = 0x00008000,
			MemoryWrite = 0x00010000
		};

		ENUM_CLASS_OPERATORS(AccessType)
		
		struct SubpassDependency
		{
			uint32 SourceSubpassIndex;
			uint32 DestinationSubpassIndex;
			PipelineStage SourceStage;
			PipelineStage DestinationStage;
			AccessType SourceAccessType;
			AccessType DestinationAccessType;
		};
		
		struct RenderPassDescription
		{
			std::vector<RenderPassAttachment> Attachments;
			std::vector<SubpassDescription> Subpasses;
			std::vector<SubpassDependency> Dependencies;
		};
		
		class HERMES_API RenderPass
		{
			ADD_DEFAULT_CONSTRUCTOR(RenderPass)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(RenderPass)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(RenderPass)
			MAKE_NON_COPYABLE(RenderPass)

		public:
			virtual uint32 SubpassCount() const = 0;
		};
	}
}
