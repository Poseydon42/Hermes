#include "VulkanPipeline.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkShaderStageFlagBits ShaderTypeToVkShaderStage(RenderInterface::ShaderType Type)
		{
			switch (Type)
			{
			case RenderInterface::ShaderType::VertexShader:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case RenderInterface::ShaderType::FragmentShader:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				HERMES_ASSERT(false);
				return (VkShaderStageFlagBits)0;
			}
		}
		
		static VkFormat VertexAttributeFormatToVkFormat(RenderInterface::VertexAttributeFormat Format)
		{
			switch(Format)
			{
				case RenderInterface::VertexAttributeFormat::Undefined:
					return VK_FORMAT_UNDEFINED;
				case RenderInterface::VertexAttributeFormat::R4G4UnsignedNormalizedPack8:
					return VK_FORMAT_R4G4_UNORM_PACK8;
				case RenderInterface::VertexAttributeFormat::R4G4B4A4UnsignedNormalizedPack16:
					return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::B4G4R4A4UnsignedNormalizedPack16:
					return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::R5G6B5UnsignedNormalizedPack16:
					return VK_FORMAT_R5G6B5_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::B5G6R5UnsignedNormalizedPack16:
					return VK_FORMAT_B5G6R5_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::R5G5B5A1UnsignedNormalizedPack16:
					return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::B5G5R5A1UnsignedNormalizedPack16:
					return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::A1R5G5B5UnsignedNormalizedPack16:
					return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
				case RenderInterface::VertexAttributeFormat::R8UnsignedNormalized:
					return VK_FORMAT_R8_UNORM;
				case RenderInterface::VertexAttributeFormat::R8SignedNormalized:
					return VK_FORMAT_R8_SNORM;
				case RenderInterface::VertexAttributeFormat::R8UnsignedScaled:
					return VK_FORMAT_R8_USCALED;
				case RenderInterface::VertexAttributeFormat::R8SignedScaled:
					return VK_FORMAT_R8_SSCALED;
				case RenderInterface::VertexAttributeFormat::R8UnsignedInteger:
					return VK_FORMAT_R8_UINT;
				case RenderInterface::VertexAttributeFormat::R8SignedInteger:
					return VK_FORMAT_R8_SINT;
				case RenderInterface::VertexAttributeFormat::R8SRGB:
					return VK_FORMAT_R8_SRGB;
				case RenderInterface::VertexAttributeFormat::R8G8UnsignedNormalized:
					return VK_FORMAT_R8G8_UNORM;
				case RenderInterface::VertexAttributeFormat::R8G8SignedNormalized:
					return VK_FORMAT_R8G8_SNORM;
				case RenderInterface::VertexAttributeFormat::R8G8UnsignedScaled:
					return VK_FORMAT_R8G8_USCALED;
				case RenderInterface::VertexAttributeFormat::R8G8SignedScaled:
					return VK_FORMAT_R8G8_SSCALED;
				case RenderInterface::VertexAttributeFormat::R8G8UnsignedInteger:
					return VK_FORMAT_R8G8_UINT;
				case RenderInterface::VertexAttributeFormat::R8G8SignedInteger:
					return VK_FORMAT_R8G8_SINT;
				case RenderInterface::VertexAttributeFormat::R8G8SRGB:
					return VK_FORMAT_R8G8_SRGB;
				case RenderInterface::VertexAttributeFormat::R8G8B8UnsignedNormalized:
					return VK_FORMAT_R8G8B8_UNORM;
				case RenderInterface::VertexAttributeFormat::R8G8B8SignedNormalized:
					return VK_FORMAT_R8G8B8_SNORM;
				case RenderInterface::VertexAttributeFormat::R8G8B8UnsignedScaled:
					return VK_FORMAT_R8G8B8_USCALED;
				case RenderInterface::VertexAttributeFormat::R8G8B8SignedScaled:
					return VK_FORMAT_R8G8B8_SSCALED;
				case RenderInterface::VertexAttributeFormat::R8G8B8UnsignedInteger:
					return VK_FORMAT_R8G8B8_UINT;
				case RenderInterface::VertexAttributeFormat::R8G8B8SignedInteger:
					return VK_FORMAT_R8G8B8_SINT;
				case RenderInterface::VertexAttributeFormat::R8G8B8SRGB:
					return VK_FORMAT_R8G8B8_SRGB;
				case RenderInterface::VertexAttributeFormat::B8G8R8UnsignedNormalized:
					return VK_FORMAT_B8G8R8_UNORM;
				case RenderInterface::VertexAttributeFormat::B8G8R8SignedNormalized:
					return VK_FORMAT_B8G8R8_SNORM;
				case RenderInterface::VertexAttributeFormat::B8G8R8UnsignedScaled:
					return VK_FORMAT_B8G8R8_USCALED;
				case RenderInterface::VertexAttributeFormat::B8G8R8SignedScaled:
					return VK_FORMAT_B8G8R8_SSCALED;
				case RenderInterface::VertexAttributeFormat::B8G8R8UnsignedInteger:
					return VK_FORMAT_B8G8R8_UINT;
				case RenderInterface::VertexAttributeFormat::B8G8R8SignedInteger:
					return VK_FORMAT_B8G8R8_SINT;
				case RenderInterface::VertexAttributeFormat::B8G8R8SRGB:
					return VK_FORMAT_B8G8R8_SRGB;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8UnsignedNormalized:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8SignedNormalized:
					return VK_FORMAT_R8G8B8A8_SNORM;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8UnsignedScaled:
					return VK_FORMAT_R8G8B8A8_USCALED;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8SignedScaled:
					return VK_FORMAT_R8G8B8A8_SSCALED;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8UnsignedInteger:
					return VK_FORMAT_R8G8B8A8_UINT;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8SignedInteger:
					return VK_FORMAT_R8G8B8A8_SINT;
				case RenderInterface::VertexAttributeFormat::R8G8B8A8SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8UnsignedNormalized:
					return VK_FORMAT_B8G8R8A8_UNORM;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8SignedNormalized:
					return VK_FORMAT_B8G8R8A8_SNORM;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8UnsignedScaled:
					return VK_FORMAT_B8G8R8A8_USCALED;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8SignedScaled:
					return VK_FORMAT_B8G8R8A8_SSCALED;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8UnsignedInteger:
					return VK_FORMAT_B8G8R8A8_UINT;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8SignedInteger:
					return VK_FORMAT_B8G8R8A8_SINT;
				case RenderInterface::VertexAttributeFormat::B8G8R8A8SRGB:
					return VK_FORMAT_B8G8R8A8_SRGB;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8UnsignedNormalizedPack32:
					return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8SignedNormalizedPack32:
					return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8UnsignedScaledPack32:
					return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8SignedScaledPack32:
					return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8UnsignedIntegerPack32:
					return VK_FORMAT_A8B8G8R8_UINT_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8SignedIntegerPack32:
					return VK_FORMAT_A8B8G8R8_SINT_PACK32;
				case RenderInterface::VertexAttributeFormat::A8B8G8R8SRGBPack32:
						return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10UnsignedNormalizedPack32:
						return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10SignedNormalizedPack32:
					return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10UnsignedScaledPack32:
					return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10SignedScaledPack32:
					return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10UnsignedIntegerPack32:
					return VK_FORMAT_A2R10G10B10_UINT_PACK32;
				case RenderInterface::VertexAttributeFormat::A2R10G10B10SignedIntegerPack32:
					return VK_FORMAT_A2R10G10B10_SINT_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10UnsignedNormalizedPack32:
					return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10SignedNormalizedPack32:
					return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10UnsignedScaledPack32:
					return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10SignedScaledPack32:
					return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10UnsignedIntegerPack32:
					return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				case RenderInterface::VertexAttributeFormat::A2B10G10R10SignedIntegerPack32:
					return VK_FORMAT_A2B10G10R10_SINT_PACK32;
				case RenderInterface::VertexAttributeFormat::R16UnsignedNormalized:
					return VK_FORMAT_R16_UNORM;
				case RenderInterface::VertexAttributeFormat::R16SignedNormalized:
					return VK_FORMAT_R16_SNORM;
				case RenderInterface::VertexAttributeFormat::R16UnsignedScaled:
					return VK_FORMAT_R16_USCALED;
				case RenderInterface::VertexAttributeFormat::R16SignedScaled:
					return VK_FORMAT_R16_SSCALED;
				case RenderInterface::VertexAttributeFormat::R16UnsignedInteger:
					return VK_FORMAT_R16_UINT;
				case RenderInterface::VertexAttributeFormat::R16SignedInteger:
					return VK_FORMAT_R16_SINT;
				case RenderInterface::VertexAttributeFormat::R16SignedFloat:
					return VK_FORMAT_R16_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R16G16UnsignedNormalized:
					return VK_FORMAT_R16G16_UNORM;
				case RenderInterface::VertexAttributeFormat::R16G16SignedNormalized:
					return VK_FORMAT_R16G16_SNORM;
				case RenderInterface::VertexAttributeFormat::R16G16UnsignedScaled:
					return VK_FORMAT_R16G16_USCALED;
				case RenderInterface::VertexAttributeFormat::R16G16SignedScaled:
					return VK_FORMAT_R16G16_SSCALED;
				case RenderInterface::VertexAttributeFormat::R16G16UnsignedInteger:
					return VK_FORMAT_R16G16_UINT;
				case RenderInterface::VertexAttributeFormat::R16G16SignedInteger:
					return VK_FORMAT_R16G16_SINT;
				case RenderInterface::VertexAttributeFormat::R16G16SignedFloat:
					return VK_FORMAT_R16G16_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R16G16B16UnsignedNormalized:
					return VK_FORMAT_R16G16B16_UNORM;
				case RenderInterface::VertexAttributeFormat::R16G16B16SignedNormalized:
					return VK_FORMAT_R16G16B16_SNORM;
				case RenderInterface::VertexAttributeFormat::R16G16B16UnsignedScaled:
					return VK_FORMAT_R16G16B16_USCALED;
				case RenderInterface::VertexAttributeFormat::R16G16B16SignedScaled:
					return VK_FORMAT_R16G16B16_SSCALED;
				case RenderInterface::VertexAttributeFormat::R16G16B16UnsignedInteger:
					return VK_FORMAT_R16G16B16_UINT;
				case RenderInterface::VertexAttributeFormat::R16G16B16SignedInteger:
					return VK_FORMAT_R16G16B16_SINT;
				case RenderInterface::VertexAttributeFormat::R16G16B16SignedFloat:
					return VK_FORMAT_R16G16B16_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16UnsignedNormalized:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16SignedNormalized:
					return VK_FORMAT_R16G16B16A16_SNORM;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16UnsignedScaled:
					return VK_FORMAT_R16G16B16A16_USCALED;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16SignedScaled:
					return VK_FORMAT_R16G16B16A16_SSCALED;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16UnsignedInteger:
					return VK_FORMAT_R16G16B16A16_UINT;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16SignedInteger:
					return VK_FORMAT_R16G16B16A16_SINT;
				case RenderInterface::VertexAttributeFormat::R16G16B16A16SignedFloat:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R32UnsignedInteger:
					return VK_FORMAT_R32_UINT;
				case RenderInterface::VertexAttributeFormat::R32SignedInteger:
					return VK_FORMAT_R32_SINT;
				case RenderInterface::VertexAttributeFormat::R32SignedFloat:
					return VK_FORMAT_R32_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R32G32UnsignedInteger:
					return VK_FORMAT_R32G32_UINT;
				case RenderInterface::VertexAttributeFormat::R32G32SignedInteger:
					return VK_FORMAT_R32G32_SINT;
				case RenderInterface::VertexAttributeFormat::R32G32SignedFloat:
					return VK_FORMAT_R32G32_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R32G32B32UnsignedInteger:
					return VK_FORMAT_R32G32B32_UINT;
				case RenderInterface::VertexAttributeFormat::R32G32B32SignedInteger:
					return VK_FORMAT_R32G32B32_SINT;
				case RenderInterface::VertexAttributeFormat::R32G32B32SignedFloat:
					return VK_FORMAT_R32G32B32_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R32G32B32A32UnsignedInteger:
					return VK_FORMAT_R32G32B32A32_UINT;
				case RenderInterface::VertexAttributeFormat::R32G32B32A32SignedInteger:
					return VK_FORMAT_R32G32B32A32_SINT;
				case RenderInterface::VertexAttributeFormat::R32G32B32A32SignedFloat:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R64UnsignedInteger:
					return VK_FORMAT_R64_UINT;
				case RenderInterface::VertexAttributeFormat::R64SignedInteger:
					return VK_FORMAT_R64_SINT;
				case RenderInterface::VertexAttributeFormat::R64SignedFloat:
					return VK_FORMAT_R64_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R64G64UnsignedInteger:
					return VK_FORMAT_R64G64_UINT;
				case RenderInterface::VertexAttributeFormat::R64G64SignedInteger:
					return VK_FORMAT_R64G64_SINT;
				case RenderInterface::VertexAttributeFormat::R64G64SignedFloat:
					return VK_FORMAT_R64G64_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R64G64B64UnsignedInteger:
					return VK_FORMAT_R64G64B64_UINT;
				case RenderInterface::VertexAttributeFormat::R64G64B64SignedInteger:
					return VK_FORMAT_R64G64B64_SINT;
				case RenderInterface::VertexAttributeFormat::R64G64B64SignedFloat:
					return VK_FORMAT_R64G64B64_SFLOAT;
				case RenderInterface::VertexAttributeFormat::R64G64B64A64UnsignedInteger:
					return VK_FORMAT_R64G64B64A64_UINT;
				case RenderInterface::VertexAttributeFormat::R64G64B64A64SignedInteger:
					return VK_FORMAT_R64G64B64A64_SINT;
				case RenderInterface::VertexAttributeFormat::R64G64B64A64SignedFloat:
					return VK_FORMAT_R64G64B64A64_SFLOAT;
				case RenderInterface::VertexAttributeFormat::B10G11R11UnsignedFloatPack32:
					return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				case RenderInterface::VertexAttributeFormat::E5B9G9R9UnsignedFloatPack32:
					return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
				case RenderInterface::VertexAttributeFormat::D16UnsignedNormalized:
					return VK_FORMAT_D16_UNORM;
				case RenderInterface::VertexAttributeFormat::X8D24UnsignedNormalizedPack32:
					return VK_FORMAT_X8_D24_UNORM_PACK32;
				case RenderInterface::VertexAttributeFormat::D32SignedFloat:
					return VK_FORMAT_D32_SFLOAT;
				case RenderInterface::VertexAttributeFormat::S8UnsignedInteger:
					return VK_FORMAT_S8_UINT;
				case RenderInterface::VertexAttributeFormat::D16UnsignedNormalizedS8UnsignedInteger:
					return VK_FORMAT_D16_UNORM_S8_UINT;
				case RenderInterface::VertexAttributeFormat::D24UnsignedNormalizedS8UnsignedInteger:
					return VK_FORMAT_D24_UNORM_S8_UINT;
				case RenderInterface::VertexAttributeFormat::D32SignedFloatS8UnsignedInteger:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
				default:
					HERMES_ASSERT(false);
					return (VkFormat)0;
			}
		}

		static VkPrimitiveTopology TopologyTypeToVkPrimitiveTopology(RenderInterface::TopologyType Type)
		{
			switch (Type)
			{
			case RenderInterface::TopologyType::PointList:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case RenderInterface::TopologyType::LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case RenderInterface::TopologyType::LineStrip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case RenderInterface::TopologyType::TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case RenderInterface::TopologyType::TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case RenderInterface::TopologyType::TriangleFan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			default: 
				HERMES_ASSERT(false);
				return (VkPrimitiveTopology)0;
			}
		}

		static VkCullModeFlags CullModeToVkCullMode(RenderInterface::CullMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case RenderInterface::CullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			case RenderInterface::CullMode::Both:
				return VK_CULL_MODE_FRONT_AND_BACK;
			default:
				HERMES_ASSERT(false);
				return (VkCullModeFlags)0;
			}
		}

		static VkPolygonMode FillModeToVkPolygonMode(RenderInterface::FillMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::FillMode::Fill:
				return VK_POLYGON_MODE_FILL;
			case RenderInterface::FillMode::Lines:
				return VK_POLYGON_MODE_LINE;
			case RenderInterface::FillMode::Points:
				return VK_POLYGON_MODE_POINT;
			default:
				HERMES_ASSERT(false);
				return (VkPolygonMode)0;
			}
		}

		static VkFrontFace FaceDirectionToVkFrontFace(RenderInterface::FaceDirection Direction)
		{
			switch (Direction)
			{
			case RenderInterface::FaceDirection::Clockwise:
				return VK_FRONT_FACE_CLOCKWISE;
			case RenderInterface::FaceDirection::CounterClockwise:
				return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			default:
				HERMES_ASSERT(false);
				return (VkFrontFace)0;
			}
		}

		static VkCompareOp ComparisonOperatorToVkCompareOp(RenderInterface::ComparisonOperator Operator)
		{
			switch (Operator)
			{
			case RenderInterface::ComparisonOperator::AlwaysFail:
				return VK_COMPARE_OP_NEVER;
			case RenderInterface::ComparisonOperator::AlwaysSucceed:
				return VK_COMPARE_OP_ALWAYS;
			case RenderInterface::ComparisonOperator::NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
			case RenderInterface::ComparisonOperator::Less:
				return VK_COMPARE_OP_LESS;
			case RenderInterface::ComparisonOperator::LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case RenderInterface::ComparisonOperator::Equal:
				return VK_COMPARE_OP_EQUAL;
			case RenderInterface::ComparisonOperator::GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case RenderInterface::ComparisonOperator::Greater:
				return VK_COMPARE_OP_GREATER;
			default:
				HERMES_ASSERT(false);
				return (VkCompareOp)0;
			}
		}
		
		VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice>& InDevice, std::shared_ptr<RenderInterface::RenderPass> InRenderPass, const RenderInterface::PipelineDescription& Description)
			: Device(InDevice)
			, RenderPass(std::reinterpret_pointer_cast<VulkanRenderPass>(InRenderPass)) // TODO : seems like a very dirty hack, maybe there's something better for this?
			, Pipelines(RenderPass->SubpassCount(), VK_NULL_HANDLE)
			, Layout(VK_NULL_HANDLE)
		{
			VkPipelineLayoutCreateInfo LayoutCreateInfo = {};
			LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			// TODO : other fields
			VK_CHECK_RESULT(vkCreatePipelineLayout(Device->GetDevice(), &LayoutCreateInfo, GVulkanAllocator, &Layout));

			std::vector<VkPipelineShaderStageCreateInfo> ShaderCreateInfos(Description.ShaderStages.size());
			for (size_t Index = 0; Index < Description.ShaderStages.size(); Index++)
			{
				ShaderCreateInfos[Index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				ShaderCreateInfos[Index].module = ((VulkanShader*)Description.ShaderStages[Index].get())->GetShader();
				ShaderCreateInfos[Index].pName = "main"; // TODO : allow user to choice or decide using reflection
				ShaderCreateInfos[Index].stage = ShaderTypeToVkShaderStage(Description.ShaderStages[Index]->GetType());
			}

			std::vector<VkVertexInputAttributeDescription> InputAttributes;
			InputAttributes.reserve(Description.VertexInput.VertexAttributes.size());
			std::vector<VkVertexInputBindingDescription> InputBindings;
			InputBindings.reserve(Description.VertexInput.VertexBindings.size());
			for (const auto& Attribute : Description.VertexInput.VertexAttributes)
			{
				VkVertexInputAttributeDescription NewAttribute = {};
				NewAttribute.location = Attribute.Location;
				NewAttribute.offset = Attribute.Offset;
				NewAttribute.binding = Attribute.BindingIndex;
				NewAttribute.format = VertexAttributeFormatToVkFormat(Attribute.Format);
				InputAttributes.push_back(NewAttribute);
			}
			for (const auto& Binding : Description.VertexInput.VertexBindings)
			{
				VkVertexInputBindingDescription NewBinding = {};
				NewBinding.binding = Binding.Index;
				NewBinding.inputRate = Binding.IsPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
				NewBinding.stride = Binding.Stride;
				InputBindings.push_back(NewBinding);
			}

			VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
			VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32)InputAttributes.size();
			VertexInputCreateInfo.pVertexAttributeDescriptions = InputAttributes.data();
			VertexInputCreateInfo.vertexBindingDescriptionCount = (uint32)InputBindings.size();
			VertexInputCreateInfo.pVertexBindingDescriptions = InputBindings.data();

			VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
			InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			InputAssemblyCreateInfo.primitiveRestartEnable = false;
			InputAssemblyCreateInfo.topology = TopologyTypeToVkPrimitiveTopology(Description.InputAssembler.Topology);

			VkPipelineViewportStateCreateInfo ViewportCreateInfo = {};
			ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			ViewportCreateInfo.viewportCount = 1;
			VkViewport Viewport = {};
			Viewport.x        = (float)Description.Viewport.Origin.X;
			Viewport.y        = (float)Description.Viewport.Origin.Y;
			Viewport.width    = (float)Description.Viewport.Dimensions.X;
			Viewport.height   = (float)Description.Viewport.Dimensions.X;
			Viewport.minDepth = 0.0f;
			Viewport.maxDepth = 1.0f;
			ViewportCreateInfo.pViewports = &Viewport;
			ViewportCreateInfo.scissorCount = 1;
			VkRect2D Scissor;
			Scissor.offset = { 0, 0 };
			Scissor.extent = { Description.Viewport.Dimensions.X, Description.Viewport.Dimensions.Y };
			ViewportCreateInfo.pScissors = &Scissor;

			VkPipelineRasterizationStateCreateInfo RasterizationCreateInfo = {};
			RasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RasterizationCreateInfo.cullMode = CullModeToVkCullMode(Description.Rasterizer.Cull);
			RasterizationCreateInfo.polygonMode = FillModeToVkPolygonMode(Description.Rasterizer.Fill);
			RasterizationCreateInfo.frontFace = FaceDirectionToVkFrontFace(Description.Rasterizer.Direction);
			RasterizationCreateInfo.lineWidth = 1.0f;

			VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo = {};
			MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = {};
			DepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			DepthStencilCreateInfo.depthTestEnable = Description.DepthStencilStage.IsDepthTestEnabled;
			DepthStencilCreateInfo.depthWriteEnable = Description.DepthStencilStage.IsDepthWriteEnabled;
			DepthStencilCreateInfo.depthCompareOp = ComparisonOperatorToVkCompareOp(Description.DepthStencilStage.ComparisonMode);
			DepthStencilCreateInfo.depthBoundsTestEnable = false;
			DepthStencilCreateInfo.stencilTestEnable = false; // TODO : implement

			std::vector<VkPipelineColorBlendStateCreateInfo> ColorBlendCreateInfos;
			std::vector<VkPipelineColorBlendAttachmentState> AttachmentBlendStates;
			uint32 AttachmentCount = 0;
			for (uint32 SubpassIndex = 0; SubpassIndex < RenderPass->SubpassCount(); SubpassIndex++)
				AttachmentCount += RenderPass->ColorAttachmentCount(SubpassIndex);
			size_t AttachmentBlendStatesIndexOffset = 0;
			ColorBlendCreateInfos.reserve(RenderPass->SubpassCount());
			// We do this to ensure that vector won't reallocate anymore, and we can get direct pointers to its elements instead
			// of calculating them after whole vector is filled
			AttachmentBlendStates.reserve(AttachmentCount);
			for (uint32 SubpassIndex = 0; SubpassIndex < RenderPass->SubpassCount(); SubpassIndex++)
			{
				VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo = {};
				ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				ColorBlendCreateInfo.logicOpEnable = false; // TODO : implement :)
				ColorBlendCreateInfo.attachmentCount = RenderPass->ColorAttachmentCount(SubpassIndex);
				ColorBlendCreateInfo.pAttachments = AttachmentBlendStates.data() + AttachmentBlendStatesIndexOffset;
				for (uint32 AttachmentIndex = 0; AttachmentIndex < RenderPass->ColorAttachmentCount(SubpassIndex); AttachmentIndex++)
				{
					VkPipelineColorBlendAttachmentState NewAttachmentBlendState = {};
					NewAttachmentBlendState.blendEnable = false;
					NewAttachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
					AttachmentBlendStates.push_back(NewAttachmentBlendState);
					AttachmentBlendStatesIndexOffset++;
				}
				ColorBlendCreateInfos.push_back(ColorBlendCreateInfo);
			}
			
			std::vector<VkGraphicsPipelineCreateInfo> PipelineCreateInfos(RenderPass->SubpassCount());
			for (uint32 i = 0; i < RenderPass->SubpassCount(); i++)
			{
				auto& CreateInfo = PipelineCreateInfos[i];
				CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				CreateInfo.stageCount = (uint32)ShaderCreateInfos.size();
				CreateInfo.pStages = ShaderCreateInfos.data();
				CreateInfo.pVertexInputState = &VertexInputCreateInfo;
				CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
				CreateInfo.pTessellationState = nullptr;
				CreateInfo.pViewportState = &ViewportCreateInfo;
				CreateInfo.pRasterizationState = &RasterizationCreateInfo;
				CreateInfo.pMultisampleState = &MultisampleCreateInfo;
				CreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
				CreateInfo.pColorBlendState = &ColorBlendCreateInfos[i];
				CreateInfo.pDynamicState = nullptr; // TODO : implement
				CreateInfo.layout = Layout;
				CreateInfo.renderPass = RenderPass->GetRenderPass();
				CreateInfo.subpass = i;
				CreateInfo.basePipelineHandle = VK_NULL_HANDLE;
				CreateInfo.basePipelineIndex = -1;
			}

			vkCreateGraphicsPipelines(Device->GetDevice(), VK_NULL_HANDLE, (uint32)PipelineCreateInfos.size(), PipelineCreateInfos.data(), GVulkanAllocator, Pipelines.data());
		}

		VulkanPipeline::~VulkanPipeline()
		{
			vkDestroyPipelineLayout(Device->GetDevice(), Layout, GVulkanAllocator);
			for (const auto& Pipeline : Pipelines)
				vkDestroyPipeline(Device->GetDevice(), Pipeline, GVulkanAllocator);
		}

		VulkanPipeline::VulkanPipeline(VulkanPipeline&& Other)
		{
			*this = std::move(Other);
		}

		VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& Other)
		{
			std::swap(Pipelines, Other.Pipelines);
			std::swap(RenderPass, Other.RenderPass);
			std::swap(Layout, Other.Layout);
			std::swap(Device, Other.Device);
			return *this;
		}
	}
}
