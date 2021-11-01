#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "Vulkan.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanDescriptorSetLayout : public RenderInterface::DescriptorSetLayout
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetLayout)

		public:
			VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings);

			~VulkanDescriptorSetLayout() override;
			VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other);
			VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& Other);

			VkDescriptorSetLayout GetDescriptorSetLayout() const { return Layout; }

		private:
			std::shared_ptr<const VulkanDevice> Device;
			VkDescriptorSetLayout Layout;
		};

		class HERMES_API VulkanDescriptorSetPool : public RenderInterface::DescriptorSetPool, public std::enable_shared_from_this<VulkanDescriptorSetPool>
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetPool)

		public:
			VulkanDescriptorSetPool(std::shared_ptr<const VulkanDevice> InDevice, uint32 NumberOfSets, const std::vector<RenderInterface::SubpoolDescription>& Subpools);

			~VulkanDescriptorSetPool() override;
			VulkanDescriptorSetPool(VulkanDescriptorSetPool&& Other);
			VulkanDescriptorSetPool& operator=(VulkanDescriptorSetPool&& Other);

			std::shared_ptr<RenderInterface::DescriptorSet> CreateDescriptorSet(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout) override;

			VkDescriptorPool GetDescriptorPool() const { return Pool; }

			uint32 GetNumberOfSets() const override { return NumSets; }

		private:
			std::shared_ptr<const VulkanDevice> Device;
			uint32 NumSets;
			VkDescriptorPool Pool;
		};

		class HERMES_API VulkanDescriptorSet : public RenderInterface::DescriptorSet
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSet)

		public:
			VulkanDescriptorSet(std::shared_ptr<const VulkanDevice> InDevice, std::shared_ptr<VulkanDescriptorSetPool> InPool, std::shared_ptr<VulkanDescriptorSetLayout> InLayout);

			~VulkanDescriptorSet() override;
			VulkanDescriptorSet(VulkanDescriptorSet&& Other);
			VulkanDescriptorSet& operator=(VulkanDescriptorSet&& Other);

			void UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Buffer& Buffer, uint32 Offset, uint32 Size) override;

			void UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Sampler& Sampler) override;

			void UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Image& Image, RenderInterface::ImageLayout LayoutAtTimeOfAccess) override;

			VkDescriptorSet GetDescriptorSet() const { return Set; }

		private:
			std::shared_ptr<const VulkanDevice> Device;
			std::shared_ptr<VulkanDescriptorSetPool> Pool;
			std::shared_ptr<VulkanDescriptorSetLayout> Layout;
			VkDescriptorSet Set;
		};
	}
}
