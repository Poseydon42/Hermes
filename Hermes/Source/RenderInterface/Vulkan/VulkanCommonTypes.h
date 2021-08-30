#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		inline VkFormat DataFormatToVkFormat(RenderInterface::DataFormat Format)
		{
			switch(Format)
			{
				case RenderInterface::DataFormat::Undefined:
					return VK_FORMAT_UNDEFINED;
				case RenderInterface::DataFormat::R4G4UnsignedNormalizedPack8:
					return VK_FORMAT_R4G4_UNORM_PACK8;
				case RenderInterface::DataFormat::R4G4B4A4UnsignedNormalizedPack16:
					return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				case RenderInterface::DataFormat::B4G4R4A4UnsignedNormalizedPack16:
					return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
				case RenderInterface::DataFormat::R5G6B5UnsignedNormalizedPack16:
					return VK_FORMAT_R5G6B5_UNORM_PACK16;
				case RenderInterface::DataFormat::B5G6R5UnsignedNormalizedPack16:
					return VK_FORMAT_B5G6R5_UNORM_PACK16;
				case RenderInterface::DataFormat::R5G5B5A1UnsignedNormalizedPack16:
					return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
				case RenderInterface::DataFormat::B5G5R5A1UnsignedNormalizedPack16:
					return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
				case RenderInterface::DataFormat::A1R5G5B5UnsignedNormalizedPack16:
					return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
				case RenderInterface::DataFormat::R8UnsignedNormalized:
					return VK_FORMAT_R8_UNORM;
				case RenderInterface::DataFormat::R8SignedNormalized:
					return VK_FORMAT_R8_SNORM;
				case RenderInterface::DataFormat::R8UnsignedScaled:
					return VK_FORMAT_R8_USCALED;
				case RenderInterface::DataFormat::R8SignedScaled:
					return VK_FORMAT_R8_SSCALED;
				case RenderInterface::DataFormat::R8UnsignedInteger:
					return VK_FORMAT_R8_UINT;
				case RenderInterface::DataFormat::R8SignedInteger:
					return VK_FORMAT_R8_SINT;
				case RenderInterface::DataFormat::R8SRGB:
					return VK_FORMAT_R8_SRGB;
				case RenderInterface::DataFormat::R8G8UnsignedNormalized:
					return VK_FORMAT_R8G8_UNORM;
				case RenderInterface::DataFormat::R8G8SignedNormalized:
					return VK_FORMAT_R8G8_SNORM;
				case RenderInterface::DataFormat::R8G8UnsignedScaled:
					return VK_FORMAT_R8G8_USCALED;
				case RenderInterface::DataFormat::R8G8SignedScaled:
					return VK_FORMAT_R8G8_SSCALED;
				case RenderInterface::DataFormat::R8G8UnsignedInteger:
					return VK_FORMAT_R8G8_UINT;
				case RenderInterface::DataFormat::R8G8SignedInteger:
					return VK_FORMAT_R8G8_SINT;
				case RenderInterface::DataFormat::R8G8SRGB:
					return VK_FORMAT_R8G8_SRGB;
				case RenderInterface::DataFormat::R8G8B8UnsignedNormalized:
					return VK_FORMAT_R8G8B8_UNORM;
				case RenderInterface::DataFormat::R8G8B8SignedNormalized:
					return VK_FORMAT_R8G8B8_SNORM;
				case RenderInterface::DataFormat::R8G8B8UnsignedScaled:
					return VK_FORMAT_R8G8B8_USCALED;
				case RenderInterface::DataFormat::R8G8B8SignedScaled:
					return VK_FORMAT_R8G8B8_SSCALED;
				case RenderInterface::DataFormat::R8G8B8UnsignedInteger:
					return VK_FORMAT_R8G8B8_UINT;
				case RenderInterface::DataFormat::R8G8B8SignedInteger:
					return VK_FORMAT_R8G8B8_SINT;
				case RenderInterface::DataFormat::R8G8B8SRGB:
					return VK_FORMAT_R8G8B8_SRGB;
				case RenderInterface::DataFormat::B8G8R8UnsignedNormalized:
					return VK_FORMAT_B8G8R8_UNORM;
				case RenderInterface::DataFormat::B8G8R8SignedNormalized:
					return VK_FORMAT_B8G8R8_SNORM;
				case RenderInterface::DataFormat::B8G8R8UnsignedScaled:
					return VK_FORMAT_B8G8R8_USCALED;
				case RenderInterface::DataFormat::B8G8R8SignedScaled:
					return VK_FORMAT_B8G8R8_SSCALED;
				case RenderInterface::DataFormat::B8G8R8UnsignedInteger:
					return VK_FORMAT_B8G8R8_UINT;
				case RenderInterface::DataFormat::B8G8R8SignedInteger:
					return VK_FORMAT_B8G8R8_SINT;
				case RenderInterface::DataFormat::B8G8R8SRGB:
					return VK_FORMAT_B8G8R8_SRGB;
				case RenderInterface::DataFormat::R8G8B8A8UnsignedNormalized:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case RenderInterface::DataFormat::R8G8B8A8SignedNormalized:
					return VK_FORMAT_R8G8B8A8_SNORM;
				case RenderInterface::DataFormat::R8G8B8A8UnsignedScaled:
					return VK_FORMAT_R8G8B8A8_USCALED;
				case RenderInterface::DataFormat::R8G8B8A8SignedScaled:
					return VK_FORMAT_R8G8B8A8_SSCALED;
				case RenderInterface::DataFormat::R8G8B8A8UnsignedInteger:
					return VK_FORMAT_R8G8B8A8_UINT;
				case RenderInterface::DataFormat::R8G8B8A8SignedInteger:
					return VK_FORMAT_R8G8B8A8_SINT;
				case RenderInterface::DataFormat::R8G8B8A8SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized:
					return VK_FORMAT_B8G8R8A8_UNORM;
				case RenderInterface::DataFormat::B8G8R8A8SignedNormalized:
					return VK_FORMAT_B8G8R8A8_SNORM;
				case RenderInterface::DataFormat::B8G8R8A8UnsignedScaled:
					return VK_FORMAT_B8G8R8A8_USCALED;
				case RenderInterface::DataFormat::B8G8R8A8SignedScaled:
					return VK_FORMAT_B8G8R8A8_SSCALED;
				case RenderInterface::DataFormat::B8G8R8A8UnsignedInteger:
					return VK_FORMAT_B8G8R8A8_UINT;
				case RenderInterface::DataFormat::B8G8R8A8SignedInteger:
					return VK_FORMAT_B8G8R8A8_SINT;
				case RenderInterface::DataFormat::B8G8R8A8SRGB:
					return VK_FORMAT_B8G8R8A8_SRGB;
				case RenderInterface::DataFormat::A8B8G8R8UnsignedNormalizedPack32:
					return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8SignedNormalizedPack32:
					return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8UnsignedScaledPack32:
					return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8SignedScaledPack32:
					return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8UnsignedIntegerPack32:
					return VK_FORMAT_A8B8G8R8_UINT_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8SignedIntegerPack32:
					return VK_FORMAT_A8B8G8R8_SINT_PACK32;
				case RenderInterface::DataFormat::A8B8G8R8SRGBPack32:
						return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10UnsignedNormalizedPack32:
						return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10SignedNormalizedPack32:
					return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10UnsignedScaledPack32:
					return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10SignedScaledPack32:
					return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10UnsignedIntegerPack32:
					return VK_FORMAT_A2R10G10B10_UINT_PACK32;
				case RenderInterface::DataFormat::A2R10G10B10SignedIntegerPack32:
					return VK_FORMAT_A2R10G10B10_SINT_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10UnsignedNormalizedPack32:
					return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10SignedNormalizedPack32:
					return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10UnsignedScaledPack32:
					return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10SignedScaledPack32:
					return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10UnsignedIntegerPack32:
					return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				case RenderInterface::DataFormat::A2B10G10R10SignedIntegerPack32:
					return VK_FORMAT_A2B10G10R10_SINT_PACK32;
				case RenderInterface::DataFormat::R16UnsignedNormalized:
					return VK_FORMAT_R16_UNORM;
				case RenderInterface::DataFormat::R16SignedNormalized:
					return VK_FORMAT_R16_SNORM;
				case RenderInterface::DataFormat::R16UnsignedScaled:
					return VK_FORMAT_R16_USCALED;
				case RenderInterface::DataFormat::R16SignedScaled:
					return VK_FORMAT_R16_SSCALED;
				case RenderInterface::DataFormat::R16UnsignedInteger:
					return VK_FORMAT_R16_UINT;
				case RenderInterface::DataFormat::R16SignedInteger:
					return VK_FORMAT_R16_SINT;
				case RenderInterface::DataFormat::R16SignedFloat:
					return VK_FORMAT_R16_SFLOAT;
				case RenderInterface::DataFormat::R16G16UnsignedNormalized:
					return VK_FORMAT_R16G16_UNORM;
				case RenderInterface::DataFormat::R16G16SignedNormalized:
					return VK_FORMAT_R16G16_SNORM;
				case RenderInterface::DataFormat::R16G16UnsignedScaled:
					return VK_FORMAT_R16G16_USCALED;
				case RenderInterface::DataFormat::R16G16SignedScaled:
					return VK_FORMAT_R16G16_SSCALED;
				case RenderInterface::DataFormat::R16G16UnsignedInteger:
					return VK_FORMAT_R16G16_UINT;
				case RenderInterface::DataFormat::R16G16SignedInteger:
					return VK_FORMAT_R16G16_SINT;
				case RenderInterface::DataFormat::R16G16SignedFloat:
					return VK_FORMAT_R16G16_SFLOAT;
				case RenderInterface::DataFormat::R16G16B16UnsignedNormalized:
					return VK_FORMAT_R16G16B16_UNORM;
				case RenderInterface::DataFormat::R16G16B16SignedNormalized:
					return VK_FORMAT_R16G16B16_SNORM;
				case RenderInterface::DataFormat::R16G16B16UnsignedScaled:
					return VK_FORMAT_R16G16B16_USCALED;
				case RenderInterface::DataFormat::R16G16B16SignedScaled:
					return VK_FORMAT_R16G16B16_SSCALED;
				case RenderInterface::DataFormat::R16G16B16UnsignedInteger:
					return VK_FORMAT_R16G16B16_UINT;
				case RenderInterface::DataFormat::R16G16B16SignedInteger:
					return VK_FORMAT_R16G16B16_SINT;
				case RenderInterface::DataFormat::R16G16B16SignedFloat:
					return VK_FORMAT_R16G16B16_SFLOAT;
				case RenderInterface::DataFormat::R16G16B16A16UnsignedNormalized:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case RenderInterface::DataFormat::R16G16B16A16SignedNormalized:
					return VK_FORMAT_R16G16B16A16_SNORM;
				case RenderInterface::DataFormat::R16G16B16A16UnsignedScaled:
					return VK_FORMAT_R16G16B16A16_USCALED;
				case RenderInterface::DataFormat::R16G16B16A16SignedScaled:
					return VK_FORMAT_R16G16B16A16_SSCALED;
				case RenderInterface::DataFormat::R16G16B16A16UnsignedInteger:
					return VK_FORMAT_R16G16B16A16_UINT;
				case RenderInterface::DataFormat::R16G16B16A16SignedInteger:
					return VK_FORMAT_R16G16B16A16_SINT;
				case RenderInterface::DataFormat::R16G16B16A16SignedFloat:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case RenderInterface::DataFormat::R32UnsignedInteger:
					return VK_FORMAT_R32_UINT;
				case RenderInterface::DataFormat::R32SignedInteger:
					return VK_FORMAT_R32_SINT;
				case RenderInterface::DataFormat::R32SignedFloat:
					return VK_FORMAT_R32_SFLOAT;
				case RenderInterface::DataFormat::R32G32UnsignedInteger:
					return VK_FORMAT_R32G32_UINT;
				case RenderInterface::DataFormat::R32G32SignedInteger:
					return VK_FORMAT_R32G32_SINT;
				case RenderInterface::DataFormat::R32G32SignedFloat:
					return VK_FORMAT_R32G32_SFLOAT;
				case RenderInterface::DataFormat::R32G32B32UnsignedInteger:
					return VK_FORMAT_R32G32B32_UINT;
				case RenderInterface::DataFormat::R32G32B32SignedInteger:
					return VK_FORMAT_R32G32B32_SINT;
				case RenderInterface::DataFormat::R32G32B32SignedFloat:
					return VK_FORMAT_R32G32B32_SFLOAT;
				case RenderInterface::DataFormat::R32G32B32A32UnsignedInteger:
					return VK_FORMAT_R32G32B32A32_UINT;
				case RenderInterface::DataFormat::R32G32B32A32SignedInteger:
					return VK_FORMAT_R32G32B32A32_SINT;
				case RenderInterface::DataFormat::R32G32B32A32SignedFloat:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case RenderInterface::DataFormat::R64UnsignedInteger:
					return VK_FORMAT_R64_UINT;
				case RenderInterface::DataFormat::R64SignedInteger:
					return VK_FORMAT_R64_SINT;
				case RenderInterface::DataFormat::R64SignedFloat:
					return VK_FORMAT_R64_SFLOAT;
				case RenderInterface::DataFormat::R64G64UnsignedInteger:
					return VK_FORMAT_R64G64_UINT;
				case RenderInterface::DataFormat::R64G64SignedInteger:
					return VK_FORMAT_R64G64_SINT;
				case RenderInterface::DataFormat::R64G64SignedFloat:
					return VK_FORMAT_R64G64_SFLOAT;
				case RenderInterface::DataFormat::R64G64B64UnsignedInteger:
					return VK_FORMAT_R64G64B64_UINT;
				case RenderInterface::DataFormat::R64G64B64SignedInteger:
					return VK_FORMAT_R64G64B64_SINT;
				case RenderInterface::DataFormat::R64G64B64SignedFloat:
					return VK_FORMAT_R64G64B64_SFLOAT;
				case RenderInterface::DataFormat::R64G64B64A64UnsignedInteger:
					return VK_FORMAT_R64G64B64A64_UINT;
				case RenderInterface::DataFormat::R64G64B64A64SignedInteger:
					return VK_FORMAT_R64G64B64A64_SINT;
				case RenderInterface::DataFormat::R64G64B64A64SignedFloat:
					return VK_FORMAT_R64G64B64A64_SFLOAT;
				case RenderInterface::DataFormat::B10G11R11UnsignedFloatPack32:
					return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				case RenderInterface::DataFormat::E5B9G9R9UnsignedFloatPack32:
					return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
				case RenderInterface::DataFormat::D16UnsignedNormalized:
					return VK_FORMAT_D16_UNORM;
				case RenderInterface::DataFormat::X8D24UnsignedNormalizedPack32:
					return VK_FORMAT_X8_D24_UNORM_PACK32;
				case RenderInterface::DataFormat::D32SignedFloat:
					return VK_FORMAT_D32_SFLOAT;
				case RenderInterface::DataFormat::S8UnsignedInteger:
					return VK_FORMAT_S8_UINT;
				case RenderInterface::DataFormat::D16UnsignedNormalizedS8UnsignedInteger:
					return VK_FORMAT_D16_UNORM_S8_UINT;
				case RenderInterface::DataFormat::D24UnsignedNormalizedS8UnsignedInteger:
					return VK_FORMAT_D24_UNORM_S8_UINT;
				case RenderInterface::DataFormat::D32SignedFloatS8UnsignedInteger:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
				default:
					HERMES_ASSERT(false);
					return (VkFormat)0;
			}
		}

		inline RenderInterface::DataFormat VkFormatToDataFormat(VkFormat Format)
		{
			switch(Format)
			{
				case VK_FORMAT_UNDEFINED:
					return RenderInterface::DataFormat::Undefined;
				case VK_FORMAT_R4G4_UNORM_PACK8:
					return RenderInterface::DataFormat::R4G4UnsignedNormalizedPack8;
				case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
					return RenderInterface::DataFormat::R4G4B4A4UnsignedNormalizedPack16;
				case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
					return RenderInterface::DataFormat::B4G4R4A4UnsignedNormalizedPack16;
				case VK_FORMAT_R5G6B5_UNORM_PACK16:
					return RenderInterface::DataFormat::R5G6B5UnsignedNormalizedPack16;
				case VK_FORMAT_B5G6R5_UNORM_PACK16:
					return RenderInterface::DataFormat::B5G6R5UnsignedNormalizedPack16;
				case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
					return RenderInterface::DataFormat::R5G5B5A1UnsignedNormalizedPack16;
				case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
					return RenderInterface::DataFormat::B5G5R5A1UnsignedNormalizedPack16;
				case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
					return RenderInterface::DataFormat::A1R5G5B5UnsignedNormalizedPack16;
				case VK_FORMAT_R8_UNORM:
					return RenderInterface::DataFormat::R8UnsignedNormalized;
				case VK_FORMAT_R8_SNORM:
					return RenderInterface::DataFormat::R8SignedNormalized;
				case VK_FORMAT_R8_USCALED:
					return RenderInterface::DataFormat::R8UnsignedScaled;
				case VK_FORMAT_R8_SSCALED:
					return RenderInterface::DataFormat::R8SignedScaled;
				case VK_FORMAT_R8_UINT:
					return RenderInterface::DataFormat::R8UnsignedInteger;
				case VK_FORMAT_R8_SINT:
					return RenderInterface::DataFormat::R8SignedInteger;
				case VK_FORMAT_R8_SRGB:
					return RenderInterface::DataFormat::R8SRGB;
				case VK_FORMAT_R8G8_UNORM:
					return RenderInterface::DataFormat::R8G8UnsignedNormalized;
				case VK_FORMAT_R8G8_SNORM:
					return RenderInterface::DataFormat::R8G8SignedNormalized;
				case VK_FORMAT_R8G8_USCALED:
					return RenderInterface::DataFormat::R8G8UnsignedScaled;
				case VK_FORMAT_R8G8_SSCALED:
					return RenderInterface::DataFormat::R8G8SignedScaled;
				case VK_FORMAT_R8G8_UINT:
					return RenderInterface::DataFormat::R8G8UnsignedInteger;
				case VK_FORMAT_R8G8_SINT:
					return RenderInterface::DataFormat::R8G8SignedInteger;
				case VK_FORMAT_R8G8_SRGB:
					return RenderInterface::DataFormat::R8G8SRGB;
				case VK_FORMAT_R8G8B8_UNORM:
					return RenderInterface::DataFormat::R8G8B8UnsignedNormalized;
				case VK_FORMAT_R8G8B8_SNORM:
					return RenderInterface::DataFormat::R8G8B8SignedNormalized;
				case VK_FORMAT_R8G8B8_USCALED:
					return RenderInterface::DataFormat::R8G8B8UnsignedScaled;
				case VK_FORMAT_R8G8B8_SSCALED:
					return RenderInterface::DataFormat::R8G8B8SignedScaled;
				case VK_FORMAT_R8G8B8_UINT:
					return RenderInterface::DataFormat::R8G8B8UnsignedInteger;
				case VK_FORMAT_R8G8B8_SINT:
					return RenderInterface::DataFormat::R8G8B8SignedInteger;
				case VK_FORMAT_R8G8B8_SRGB:
					return RenderInterface::DataFormat::R8G8B8SRGB;
				case VK_FORMAT_B8G8R8_UNORM:
					return RenderInterface::DataFormat::B8G8R8UnsignedNormalized;
				case VK_FORMAT_B8G8R8_SNORM:
					return RenderInterface::DataFormat::B8G8R8SignedNormalized;
				case VK_FORMAT_B8G8R8_USCALED:
					return RenderInterface::DataFormat::B8G8R8UnsignedScaled;
				case VK_FORMAT_B8G8R8_SSCALED:
					return RenderInterface::DataFormat::B8G8R8SignedScaled;
				case VK_FORMAT_B8G8R8_UINT:
					return RenderInterface::DataFormat::B8G8R8UnsignedInteger;
				case VK_FORMAT_B8G8R8_SINT:
					return RenderInterface::DataFormat::B8G8R8SignedInteger;
				case VK_FORMAT_B8G8R8_SRGB:
					return RenderInterface::DataFormat::B8G8R8SRGB;
				case VK_FORMAT_R8G8B8A8_UNORM:
					return RenderInterface::DataFormat::R8G8B8A8UnsignedNormalized;
				case VK_FORMAT_R8G8B8A8_SNORM:
					return RenderInterface::DataFormat::R8G8B8A8SignedNormalized;
				case VK_FORMAT_R8G8B8A8_USCALED:
					return RenderInterface::DataFormat::R8G8B8A8UnsignedScaled;
				case VK_FORMAT_R8G8B8A8_SSCALED:
					return RenderInterface::DataFormat::R8G8B8A8SignedScaled;
				case VK_FORMAT_R8G8B8A8_UINT:
					return RenderInterface::DataFormat::R8G8B8A8UnsignedInteger;
				case VK_FORMAT_R8G8B8A8_SINT:
					return RenderInterface::DataFormat::R8G8B8A8SignedInteger;
				case VK_FORMAT_R8G8B8A8_SRGB:
					return RenderInterface::DataFormat::R8G8B8A8SRGB;
				case VK_FORMAT_B8G8R8A8_UNORM:
					return RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
				case VK_FORMAT_B8G8R8A8_SNORM:
					return RenderInterface::DataFormat::B8G8R8A8SignedNormalized;
				case VK_FORMAT_B8G8R8A8_USCALED:
					return RenderInterface::DataFormat::B8G8R8A8UnsignedScaled;
				case VK_FORMAT_B8G8R8A8_SSCALED:
					return RenderInterface::DataFormat::B8G8R8A8SignedScaled;
				case VK_FORMAT_B8G8R8A8_UINT:
					return RenderInterface::DataFormat::B8G8R8A8UnsignedInteger;
				case VK_FORMAT_B8G8R8A8_SINT:
					return RenderInterface::DataFormat::B8G8R8A8SignedInteger;
				case VK_FORMAT_B8G8R8A8_SRGB:
					return RenderInterface::DataFormat::B8G8R8A8SRGB;
				case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8UnsignedNormalizedPack32;
				case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8SignedNormalizedPack32;
				case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8UnsignedScaledPack32;
				case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8SignedScaledPack32;
				case VK_FORMAT_A8B8G8R8_UINT_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8UnsignedIntegerPack32;
				case VK_FORMAT_A8B8G8R8_SINT_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8SignedIntegerPack32;
				case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
					return RenderInterface::DataFormat::A8B8G8R8SRGBPack32;
				case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10UnsignedNormalizedPack32;
				case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10SignedNormalizedPack32;
				case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10UnsignedScaledPack32;
				case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10SignedScaledPack32;
				case VK_FORMAT_A2R10G10B10_UINT_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10UnsignedIntegerPack32;
				case VK_FORMAT_A2R10G10B10_SINT_PACK32:
					return RenderInterface::DataFormat::A2R10G10B10SignedIntegerPack32;
				case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10UnsignedNormalizedPack32;
				case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10SignedNormalizedPack32;
				case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10UnsignedScaledPack32;
				case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10SignedScaledPack32;
				case VK_FORMAT_A2B10G10R10_UINT_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10UnsignedIntegerPack32;
				case VK_FORMAT_A2B10G10R10_SINT_PACK32:
					return RenderInterface::DataFormat::A2B10G10R10SignedIntegerPack32;
				case VK_FORMAT_R16_UNORM:
					return RenderInterface::DataFormat::R16UnsignedNormalized;
				case VK_FORMAT_R16_SNORM:
					return RenderInterface::DataFormat::R16SignedNormalized;
				case VK_FORMAT_R16_USCALED:
					return RenderInterface::DataFormat::R16UnsignedScaled;
				case VK_FORMAT_R16_SSCALED:
					return RenderInterface::DataFormat::R16SignedScaled;
				case VK_FORMAT_R16_UINT:
					return RenderInterface::DataFormat::R16UnsignedInteger;
				case VK_FORMAT_R16_SINT:
					return RenderInterface::DataFormat::R16SignedInteger;
				case VK_FORMAT_R16_SFLOAT:
					return RenderInterface::DataFormat::R16SignedFloat;
				case VK_FORMAT_R16G16_UNORM:
					return RenderInterface::DataFormat::R16G16UnsignedNormalized;
				case VK_FORMAT_R16G16_SNORM:
					return RenderInterface::DataFormat::R16G16SignedNormalized;
				case VK_FORMAT_R16G16_USCALED:
					return RenderInterface::DataFormat::R16G16UnsignedScaled;
				case VK_FORMAT_R16G16_SSCALED:
					return RenderInterface::DataFormat::R16G16SignedScaled;
				case VK_FORMAT_R16G16_UINT:
					return RenderInterface::DataFormat::R16G16UnsignedInteger;
				case VK_FORMAT_R16G16_SINT:
					return RenderInterface::DataFormat::R16G16SignedInteger;
				case VK_FORMAT_R16G16_SFLOAT:
					return RenderInterface::DataFormat::R16G16SignedFloat;
				case VK_FORMAT_R16G16B16_UNORM:
					return RenderInterface::DataFormat::R16G16B16UnsignedNormalized;
				case VK_FORMAT_R16G16B16_SNORM:
					return RenderInterface::DataFormat::R16G16B16SignedNormalized;
				case VK_FORMAT_R16G16B16_USCALED:
					return RenderInterface::DataFormat::R16G16B16UnsignedScaled;
				case VK_FORMAT_R16G16B16_SSCALED:
					return RenderInterface::DataFormat::R16G16B16SignedScaled;
				case VK_FORMAT_R16G16B16_UINT:
					return RenderInterface::DataFormat::R16G16B16UnsignedInteger;
				case VK_FORMAT_R16G16B16_SINT:
					return RenderInterface::DataFormat::R16G16B16SignedInteger;
				case VK_FORMAT_R16G16B16_SFLOAT:
					return RenderInterface::DataFormat::R16G16B16SignedFloat;
				case VK_FORMAT_R16G16B16A16_UNORM:
					return RenderInterface::DataFormat::R16G16B16A16UnsignedNormalized;
				case VK_FORMAT_R16G16B16A16_SNORM:
					return RenderInterface::DataFormat::R16G16B16A16SignedNormalized;
				case VK_FORMAT_R16G16B16A16_USCALED:
					return RenderInterface::DataFormat::R16G16B16A16UnsignedScaled;
				case VK_FORMAT_R16G16B16A16_SSCALED:
					return RenderInterface::DataFormat::R16G16B16A16SignedScaled;
				case VK_FORMAT_R16G16B16A16_UINT:
					return RenderInterface::DataFormat::R16G16B16A16UnsignedInteger;
				case VK_FORMAT_R16G16B16A16_SINT:
					return RenderInterface::DataFormat::R16G16B16A16SignedInteger;
				case VK_FORMAT_R16G16B16A16_SFLOAT:
					return RenderInterface::DataFormat::R16G16B16A16SignedFloat;
				case VK_FORMAT_R32_UINT:
					return RenderInterface::DataFormat::R32UnsignedInteger;
				case VK_FORMAT_R32_SINT:
					return RenderInterface::DataFormat::R32SignedInteger;
				case VK_FORMAT_R32_SFLOAT:
					return RenderInterface::DataFormat::R32SignedFloat;
				case VK_FORMAT_R32G32_UINT:
					return RenderInterface::DataFormat::R32G32UnsignedInteger;
				case VK_FORMAT_R32G32_SINT:
					return RenderInterface::DataFormat::R32G32SignedInteger;
				case VK_FORMAT_R32G32_SFLOAT:
					return RenderInterface::DataFormat::R32G32SignedFloat;
				case VK_FORMAT_R32G32B32_UINT:
					return RenderInterface::DataFormat::R32G32B32UnsignedInteger;
				case VK_FORMAT_R32G32B32_SINT:
					return RenderInterface::DataFormat::R32G32B32SignedInteger;
				case VK_FORMAT_R32G32B32_SFLOAT:
					return RenderInterface::DataFormat::R32G32B32SignedFloat;
				case VK_FORMAT_R32G32B32A32_UINT:
					return RenderInterface::DataFormat::R32G32B32A32UnsignedInteger;
				case VK_FORMAT_R32G32B32A32_SINT:
					return RenderInterface::DataFormat::R32G32B32A32SignedInteger;
				case VK_FORMAT_R32G32B32A32_SFLOAT:
					return RenderInterface::DataFormat::R32G32B32A32SignedFloat;
				case VK_FORMAT_R64_UINT:
					return RenderInterface::DataFormat::R64UnsignedInteger;
				case VK_FORMAT_R64_SINT:
					return RenderInterface::DataFormat::R64SignedInteger;
				case VK_FORMAT_R64_SFLOAT:
					return RenderInterface::DataFormat::R64SignedFloat;
				case VK_FORMAT_R64G64_UINT:
					return RenderInterface::DataFormat::R64G64UnsignedInteger;
				case VK_FORMAT_R64G64_SINT:
					return RenderInterface::DataFormat::R64G64SignedInteger;
				case VK_FORMAT_R64G64_SFLOAT:
					return RenderInterface::DataFormat::R64G64SignedFloat;
				case VK_FORMAT_R64G64B64_UINT:
					return RenderInterface::DataFormat::R64G64B64UnsignedInteger;
				case VK_FORMAT_R64G64B64_SINT:
					return RenderInterface::DataFormat::R64G64B64SignedInteger;
				case VK_FORMAT_R64G64B64_SFLOAT:
					return RenderInterface::DataFormat::R64G64B64SignedFloat;
				case VK_FORMAT_R64G64B64A64_UINT:
					return RenderInterface::DataFormat::R64G64B64A64UnsignedInteger;
				case VK_FORMAT_R64G64B64A64_SINT:
					return RenderInterface::DataFormat::R64G64B64A64SignedInteger;
				case VK_FORMAT_R64G64B64A64_SFLOAT:
					return RenderInterface::DataFormat::R64G64B64A64SignedFloat;
				case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
					return RenderInterface::DataFormat::B10G11R11UnsignedFloatPack32;
				case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
					return RenderInterface::DataFormat::E5B9G9R9UnsignedFloatPack32;
				case VK_FORMAT_D16_UNORM:
					return RenderInterface::DataFormat::D16UnsignedNormalized;
				case VK_FORMAT_X8_D24_UNORM_PACK32:
					return RenderInterface::DataFormat::X8D24UnsignedNormalizedPack32;
				case VK_FORMAT_D32_SFLOAT:
					return RenderInterface::DataFormat::D32SignedFloat;
				case VK_FORMAT_S8_UINT:
					return RenderInterface::DataFormat::S8UnsignedInteger;
				case VK_FORMAT_D16_UNORM_S8_UINT:
					return RenderInterface::DataFormat::D16UnsignedNormalizedS8UnsignedInteger;
				case VK_FORMAT_D24_UNORM_S8_UINT:
					return RenderInterface::DataFormat::D24UnsignedNormalizedS8UnsignedInteger;
				case VK_FORMAT_D32_SFLOAT_S8_UINT:
					return RenderInterface::DataFormat::D32SignedFloatS8UnsignedInteger;
				default:
					HERMES_ASSERT(false);
					return (RenderInterface::DataFormat)0;
			}
		}

		inline VkImageLayout ImageLayoutToVkImageLayout(RenderInterface::ImageLayout Layout)
		{
			switch (Layout)
			{
			case RenderInterface::ImageLayout::Undefined:
				return VK_IMAGE_LAYOUT_UNDEFINED;
			case RenderInterface::ImageLayout::General:
				return VK_IMAGE_LAYOUT_GENERAL;
			case RenderInterface::ImageLayout::Preinitialized:
				return VK_IMAGE_LAYOUT_PREINITIALIZED;
			case RenderInterface::ImageLayout::ReadyForPresentation:
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			case RenderInterface::ImageLayout::ColorAttachmentOptimal:
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case RenderInterface::ImageLayout::DepthAttachmentOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case RenderInterface::ImageLayout::DepthReadOnlyOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			case RenderInterface::ImageLayout::StencilAttachmentOptimal:
				return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			case RenderInterface::ImageLayout::StencilReadOnlyOptimal:
				return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
			case RenderInterface::ImageLayout::DepthStencilAttachmentOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case RenderInterface::ImageLayout::DepthStencilReadOnlyOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case RenderInterface::ImageLayout::DepthAttachmentStencilReadOnlyOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			case RenderInterface::ImageLayout::DepthReadOnlyStencilAttachmentOptimal:
				return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
			case RenderInterface::ImageLayout::ShaderReadOnlyOptimal:
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case RenderInterface::ImageLayout::TransferDestinationOptimal:
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case RenderInterface::ImageLayout::TransferSourceOptimal:
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			default:
				HERMES_ASSERT(false);
				return (VkImageLayout)0;
			}
		}

		inline VkImageAspectFlags VkAspectFlagsFromVkFormat(VkFormat Format)
		{
			switch(Format)
			{
				case VK_FORMAT_R4G4_UNORM_PACK8:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R5G6B5_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B5G6R5_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R8G8B8A8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B8G8R8A8_SRGB:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_UINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_SINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_UINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2R10G10B10_SINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_UINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_A2B10G10R10_SINT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_UNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_SNORM:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_USCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_SSCALED:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R16G16B16A16_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32A32_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32A32_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R32G32B32A32_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64A64_UINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64A64_SINT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_R64G64B64A64_SFLOAT:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
					return VK_IMAGE_ASPECT_COLOR_BIT;
				case VK_FORMAT_D16_UNORM:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
				case VK_FORMAT_X8_D24_UNORM_PACK32:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
				case VK_FORMAT_D32_SFLOAT:
					return VK_IMAGE_ASPECT_DEPTH_BIT;
				case VK_FORMAT_S8_UINT:
					return VK_IMAGE_ASPECT_STENCIL_BIT;
				case VK_FORMAT_D16_UNORM_S8_UINT:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				case VK_FORMAT_D24_UNORM_S8_UINT:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				case VK_FORMAT_D32_SFLOAT_S8_UINT:
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				case VK_FORMAT_UNDEFINED:
				default:
					HERMES_ASSERT(false);
					return (VkImageAspectFlags)0;
			}
		}
	}
}

