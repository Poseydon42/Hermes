#include "MaterialInstance.h"

#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	void MaterialInstance::PrepareForRender() const
	{
		if (IsDirty)
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
		: BaseMaterial(std::move(Material))
	{
		CPUBuffer.resize(UniformBufferSize);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		UniformBuffer = Device.CreateBuffer(UniformBufferSize,
		                                    RenderInterface::BufferUsageType::UniformBuffer |
		                                    RenderInterface::BufferUsageType::CPUAccessible);
		DescriptorSet = DescriptorAllocator.Allocate(BaseMaterial->GetDescriptorSetLayout());
		DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));

		// Filling CPU buffer with default values
		for (const auto& Property : BaseMaterial->GetProperties())
		{
			auto PropertySize = GetMaterialPropertySize(Property.Type);
			HERMES_ASSERT(Property.Offset + PropertySize <= UniformBufferSize);
			memcpy(CPUBuffer.data() + Property.Offset, &Property.DefaultValue, PropertySize);
		}
		IsDirty = true;
	}
}
