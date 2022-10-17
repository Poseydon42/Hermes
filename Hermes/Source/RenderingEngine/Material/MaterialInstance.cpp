#include "MaterialInstance.h"

#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Texture.h"
#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	void MaterialInstance::SetTextureProperty(const String& Name, const Texture& Value)
	{
		auto* Property = BaseMaterial->FindProperty(Name);
		HERMES_ASSERT(Property);

		DescriptorSet->UpdateWithImageAndSampler(Property->Binding, 0, Value.GetDefaultView(),
		                                         Renderer::Get().GetDefaultSampler(),
		                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
	}

	void MaterialInstance::PrepareForRender() const
	{
		if (IsDirty && HasUniformBuffer)
		{
			// TODO : use frame local allocator for better performance
			auto BufferSize = UniformBuffer->GetSize();
			auto* MappedMemory = UniformBuffer->Map();
			HERMES_ASSERT(MappedMemory);
			memcpy(MappedMemory, CPUBuffer.data(), BufferSize);
			UniformBuffer->Unmap();

			IsDirty = false;
		}
	}

	const Material& MaterialInstance::GetBaseMaterial() const
	{
		return *BaseMaterial;
	}

	const RenderInterface::DescriptorSet& MaterialInstance::GetMaterialDescriptorSet() const
	{
		return *DescriptorSet;
	}

	MaterialInstance::MaterialInstance(std::shared_ptr<const Material> Material, size_t UniformBufferSize)
		: HasUniformBuffer(UniformBufferSize > 0)
		, BaseMaterial(std::move(Material))
	{
		CPUBuffer.resize(UniformBufferSize);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		DescriptorSet = DescriptorAllocator.Allocate(BaseMaterial->GetDescriptorSetLayout());

		if (HasUniformBuffer)
		{
			UniformBuffer = Device.CreateBuffer(UniformBufferSize,
			                                    RenderInterface::BufferUsageType::UniformBuffer |
			                                    RenderInterface::BufferUsageType::CPUAccessible);
			DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));
			IsDirty = true;
		}		
	}
}
