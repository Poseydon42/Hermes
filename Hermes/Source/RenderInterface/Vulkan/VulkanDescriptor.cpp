#include "VulkanDescriptor.h"

#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings)
			: Device(std::move(InDevice))
			, Layout(VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> VulkanBindings(Bindings.size());
			auto VulkanBinding = VulkanBindings.begin();
			for (const auto& Binding : Bindings)
			{
				VulkanBinding->binding = Binding.Index;
				VulkanBinding->descriptorCount = Binding.DescriptorCount; // TODO
				VulkanBinding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				VulkanBinding->pImmutableSamplers = 0;
				VulkanBinding->stageFlags = ShaderTypeToVkShaderStage(Binding.Shader);
				++VulkanBinding;
			}

			VkDescriptorSetLayoutCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.bindingCount = static_cast<uint32>(VulkanBindings.size());
			CreateInfo.pBindings = VulkanBindings.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Layout));
		}

		VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
		{
			vkDestroyDescriptorSetLayout(Device->GetDevice(), Layout, GVulkanAllocator);
		}

		VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Layout, Other.Layout);

			return *this;
		}

		VulkanDescriptorSetPool::VulkanDescriptorSetPool(std::shared_ptr<VulkanDevice> InDevice, uint32 Size)
			: Device(std::move(InDevice))
			, MaxSize(Size)
			, Pool(VK_NULL_HANDLE)
		{
			VkDescriptorPoolCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			CreateInfo.maxSets = Size;
			VkDescriptorPoolSize PoolSize = {};
			PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO : fix
			PoolSize.descriptorCount = Size;
			CreateInfo.pPoolSizes = &PoolSize;
			CreateInfo.poolSizeCount = 1;

			VK_CHECK_RESULT(vkCreateDescriptorPool(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Pool))
		}

		VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
		{
			vkDestroyDescriptorPool(Device->GetDevice(), Pool, GVulkanAllocator);
		}

		VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanDescriptorSetPool&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSetPool& VulkanDescriptorSetPool::operator=(VulkanDescriptorSetPool&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(MaxSize, Other.MaxSize);
			std::swap(Pool, Other.Pool);

			return *this;
		}
	}
}
