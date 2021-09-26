#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanDescriptorSetLayout : public RenderInterface::DescriptorSetLayout
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetLayout)

		public:
			VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings);

			~VulkanDescriptorSetLayout() override;
			VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other);
			VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& Other);

			VkDescriptorSetLayout GetDescriptorSetLayout() const { return Layout; }

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkDescriptorSetLayout Layout;
		};

		class HERMES_API VulkanDescriptorSetPool : public RenderInterface::DescriptorSetPool, public std::enable_shared_from_this<VulkanDescriptorSetPool>
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetPool)

		public:
			VulkanDescriptorSetPool(std::shared_ptr<VulkanDevice> InDevice, uint32 Size);

			~VulkanDescriptorSetPool() override;
			VulkanDescriptorSetPool(VulkanDescriptorSetPool&& Other);
			VulkanDescriptorSetPool& operator=(VulkanDescriptorSetPool&& Other);

			std::shared_ptr<RenderInterface::DescriptorSet> CreateDescriptorSet(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout) override;

			uint32 GetSize() const override { return MaxSize; }

			VkDescriptorPool GetDescriptorPool() const { return Pool; }

		private:
			std::shared_ptr<VulkanDevice> Device;
			uint32 MaxSize;
			VkDescriptorPool Pool;
		};

		class HERMES_API VulkanDescriptorSet : public RenderInterface::DescriptorSet
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSet)

		public:
			VulkanDescriptorSet(std::shared_ptr<VulkanDevice> InDevice, std::shared_ptr<VulkanDescriptorSetPool> InPool, std::shared_ptr<VulkanDescriptorSetLayout> InLayout);

			~VulkanDescriptorSet() override;
			VulkanDescriptorSet(VulkanDescriptorSet&& Other);
			VulkanDescriptorSet& operator=(VulkanDescriptorSet&& Other);

			void UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Buffer& Buffer, uint32 Offset, uint32 Size) override;

			VkDescriptorSet GetDescriptorSet() const { return Set; }
			
		private:
			std::shared_ptr<VulkanDevice> Device;
			std::shared_ptr<VulkanDescriptorSetPool> Pool;
			std::shared_ptr<VulkanDescriptorSetLayout> Layout;
			VkDescriptorSet Set;
		};
	}
}
