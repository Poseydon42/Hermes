#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	enum class CoordinateSystem
	{
		Normalized,
		Unnormalized
	};

	struct SamplerDescription
	{
		VkFilter MagnificationFilter;
		VkFilter MinificationFilter;

		VkSamplerMipmapMode MipmapMode;

		VkSamplerAddressMode AddressMode;
		CoordinateSystem CoordinateSystem;

		bool Anisotropy;
		float AnisotropyLevel;

		float MinLOD;
		float MaxLOD;
		float LODBias;
	};

	/*
	 * A safe wrapper around VkSampler
	 */
	class HERMES_API Sampler
	{
		MAKE_NON_COPYABLE(Sampler)
		MAKE_NON_MOVABLE(Sampler)

	public:
		Sampler(std::shared_ptr<Device::VkDeviceHolder> InDevice, const SamplerDescription& Description);

		~Sampler();

		VkSampler GetSampler() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkSampler Handle = VK_NULL_HANDLE;
	};
}
