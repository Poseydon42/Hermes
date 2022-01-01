#include "VulkanSampler.h"

#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkSamplerAddressMode AddressingModeToVkSamplerAddressMode(RenderInterface::AddressingMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::AddressingMode::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case RenderInterface::AddressingMode::RepeatMirrored:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case RenderInterface::AddressingMode::ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			default:
				HERMES_ASSERT(false);
				return static_cast<VkSamplerAddressMode>(0);
			}
		}

		static VkSamplerMipmapMode MipmappingModeToVkSamplerMipmapMode(RenderInterface::MipmappingMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::MipmappingMode::Linear:
				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			case RenderInterface::MipmappingMode::Nearest:
				return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			default:
				HERMES_ASSERT(false);
				return static_cast<VkSamplerMipmapMode>(0);
			}
		}

		VulkanSampler::VulkanSampler(std::shared_ptr<const VulkanDevice> InDevice, const RenderInterface::SamplerDescription& Description)
			: Device(std::move(InDevice))
			, Sampler(VK_NULL_HANDLE)
		{
			VkSamplerCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			CreateInfo.addressModeU = AddressingModeToVkSamplerAddressMode(Description.AddressingModeU);
			CreateInfo.addressModeV = AddressingModeToVkSamplerAddressMode(Description.AddressingModeV);
			CreateInfo.magFilter = FilteringModeToVkFilter(Description.MagnificationFilteringMode);
			CreateInfo.minFilter = FilteringModeToVkFilter(Description.MinificationFilteringMode);
			if (Description.CoordinateSystem == RenderInterface::CoordinateSystem::Normalized)
				CreateInfo.unnormalizedCoordinates = VK_FALSE;
			else if (Description.CoordinateSystem == RenderInterface::CoordinateSystem::Unnormalized)
				CreateInfo.unnormalizedCoordinates = VK_TRUE;
			else
				HERMES_ASSERT(false);
			CreateInfo.compareEnable = VK_FALSE; // TODO
			CreateInfo.anisotropyEnable = Description.AnisotropyLevel.has_value();
			CreateInfo.maxAnisotropy = Description.AnisotropyLevel.value_or(0.0f);
			CreateInfo.minLod = Description.MinMipLevel;
			CreateInfo.maxLod = Description.MaxMipLevel;
			CreateInfo.mipLodBias = Description.MipBias;
			CreateInfo.mipmapMode = MipmappingModeToVkSamplerMipmapMode(Description.MipMode);

			VK_CHECK_RESULT(vkCreateSampler(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Sampler));
		}

		VulkanSampler::~VulkanSampler()
		{
			vkDestroySampler(Device->GetDevice(), Sampler, GVulkanAllocator);
		}

		VulkanSampler::VulkanSampler(VulkanSampler&& Other)
		{
			*this = std::move(Other);
		}

		VulkanSampler& VulkanSampler::operator=(VulkanSampler&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Sampler, Other.Sampler);

			return *this;
		}

		VkSampler VulkanSampler::GetSampler() const
		{
			return Sampler;
		}
	}
}
