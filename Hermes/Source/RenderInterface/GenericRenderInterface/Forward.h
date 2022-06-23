#pragma once

namespace Hermes
{
	namespace RenderInterface
	{
		class Buffer;
		class CommandBuffer;
		class DescriptorSet;
		class DescriptorSetLayout;
		class DescriptorSetPool;
		class Device;
		class Fence;
		class Image;
		class ImageView;
		class Instance;
		class PhysicalDevice;
		class Pipeline;
		class Queue;
		class RenderPass;
		class RenderTarget;
		class Sampler;
		class Shader;
		class ShaderBase;
		class Swapchain;

		struct BufferCopyRegion;
		struct BufferMemoryBarrier;
		struct BufferToImageCopyRegion;
		struct ClearColor;
		struct ColorBlendingDescription;
		struct DepthStencilDescription;
		struct DescriptorBinding;
		struct DeviceProperties;
		struct ImageBlitRegion;
		struct ImageCopyRegion;
		struct ImageMemoryBarrier;
		struct ImageViewDescription;
		struct InputAssemblerDescription;
		struct PipelineDescription;
		struct PushConstantRange;
		struct RenderPassAttachment;
		struct RasterizerDescription;
		struct SamplerDescription;
		struct SubpoolDescription;
		struct VertexAttribute;
		struct VertexBinding;
		struct VertexInputDescription;
		struct ViewportDescription;

		enum class AccessType;
		enum class AddressingMode;
		enum class AttachmentLoadOp;
		enum class AttachmentStoreOp;
		enum class AttachmentType;
		enum class BufferUsageType;
		enum class ComparisonOperator;
		enum class CoordinateSystem;
		enum class CullMode;
		enum class DataFormat;
		enum class DescriptorType;
		enum class FaceDirection;
		enum class FillMode;
		enum class FilteringMode;
		enum class ImageAspect;
		enum class ImageLayout;
		enum class ImageUsageType;
		enum class IndexSize;
		enum class MipmappingMode;
		enum class PipelineStage;
		enum class QueueType;
		enum class ShaderType;
		enum class TopologyType;
	}
}