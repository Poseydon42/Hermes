#include "Sampler.h"

namespace Hermes::Vulkan
{
	Sampler::Sampler(std::shared_ptr<Device::VkDeviceHolder> InDevice, const SamplerDescription& Description)
		: Device(std::move(InDevice))
	{
		VkSamplerCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		CreateInfo.addressModeU = Description.AddressMode;
		CreateInfo.addressModeV = Description.AddressMode;
		CreateInfo.addressModeW = Description.AddressMode;
		CreateInfo.magFilter = Description.MagnificationFilter;
		CreateInfo.minFilter = Description.MinificationFilter;
		if (Description.CoordinateSystem == CoordinateSystem::Unnormalized)
			CreateInfo.unnormalizedCoordinates = true;
		else
			CreateInfo.unnormalizedCoordinates = false;
		CreateInfo.compareEnable = VK_FALSE; // TODO
		CreateInfo.anisotropyEnable = Description.Anisotropy;
		CreateInfo.maxAnisotropy = Description.AnisotropyLevel;
		CreateInfo.minLod = Description.MinLOD;
		CreateInfo.maxLod = Description.MaxLOD;
		CreateInfo.mipLodBias = Description.LODBias;
		CreateInfo.mipmapMode = Description.MipmapMode;

		VK_CHECK_RESULT(vkCreateSampler(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
	}

	Sampler::~Sampler()
	{
		vkDestroySampler(Device->Device, Handle, GVulkanAllocator);
	}

	VkSampler Sampler::GetSampler() const
	{
		return Handle;
	}
}
