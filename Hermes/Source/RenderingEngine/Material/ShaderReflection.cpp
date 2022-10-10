#include "ShaderReflection.h"

#include <spirv_cross.hpp>

#include "Core/Misc/StringUtils.h"
#include "Logging/Logger.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes
{
	static MaterialPropertyDataType SPIRVTypeToMaterialPropertyDataType(spirv_cross::SPIRType::BaseType Type)
	{
		switch (Type)
		{
		case spirv_cross::SPIRType::Float:
			return MaterialPropertyDataType::Float;
		default:
			HERMES_ASSERT_LOG(false, L"Unsupported SPIR-V type");
			return MaterialPropertyDataType::Undefined;
		}
	}

	ShaderReflection::ShaderReflection(const String& ShaderPath)
	{
		auto ShaderFile = PlatformFilesystem::OpenFile(ShaderPath, IPlatformFile::FileAccessMode::Read,
		                                               IPlatformFile::FileOpenMode::OpenExisting);
		HERMES_ASSERT(ShaderFile);
		HERMES_ASSERT(ShaderFile->Size() % 4 == 0); // NOTE : SPIR-V binary size must always be divisible by 4

		std::vector<uint32> ShaderCode(ShaderFile->Size() / 4);
		HERMES_ASSERT(ShaderFile->Read(reinterpret_cast<uint8*>(ShaderCode.data()), ShaderFile->Size()));

		spirv_cross::Compiler Compiler(std::move(ShaderCode));
		const auto& UniformBuffers = Compiler.get_shader_resources().uniform_buffers;

		const spirv_cross::Resource* MaterialDataUBO = nullptr;
		for (const auto& Buffer : UniformBuffers)
		{
			// NOTE : at the moment only a single uniform buffer at descriptor set 0 binding 0 is supported
			if (Compiler.get_decoration(Buffer.id, spv::DecorationDescriptorSet) == 1 &&
				Compiler.get_decoration(Buffer.id, spv::DecorationBinding) == 0)
			{
				MaterialDataUBO = &Buffer;
				break;
			}
		}
		HERMES_ASSERT(MaterialDataUBO);

		// Numeric properties (in UBO at binding 0)
		const auto& TypeContainer = Compiler.get_type(MaterialDataUBO->base_type_id);
		SizeForUniformBuffer = Compiler.get_declared_struct_size(TypeContainer);
		for (uint32 MemberIndex = 0; MemberIndex < TypeContainer.member_types.size(); MemberIndex++)
		{
			const auto& ANSIName = Compiler.get_member_name(TypeContainer.self, MemberIndex);
			auto Name = StringUtils::ANSIToString(ANSIName);

			auto& NativeType = Compiler.get_type(TypeContainer.member_types[MemberIndex]);
			auto DataType = SPIRVTypeToMaterialPropertyDataType(NativeType.basetype);

			auto Size = Compiler.get_declared_struct_member_size(TypeContainer, MemberIndex);
			auto Offset = Compiler.type_struct_member_offset(TypeContainer, MemberIndex);

			auto Type = MaterialPropertyType::Undefined;
			if (NativeType.vecsize == 1 && NativeType.columns == 1)
			{
				Type = MaterialPropertyType::Value;
			}
			else if (NativeType.vecsize > 1 && NativeType.columns == 1)
			{
				Type = MaterialPropertyType::Vector;
			}
			else
			{
				HERMES_ASSERT_LOG(NativeType.vecsize == NativeType.columns,
				                  L"Reflection of non-square matrices is not supported");
				Type = MaterialPropertyType::Matrix;
			}

			MaterialProperty Property;
			Property.Type = Type;
			Property.DataType = DataType;
			Property.Width = NativeType.vecsize;
			Property.Size = Size;
			Property.Offset = Offset;
			Property.Binding = 0;

			Properties[std::move(Name)] = Property;
		}

		// Texture property (DescriptorType::CombinedImageSampler)
		for (const auto& Texture : Compiler.get_shader_resources().sampled_images)
		{
			// NOTE: material properties are located only in descriptor set 1
			if (Compiler.get_decoration(Texture.id, spv::DecorationDescriptorSet) != 1)
				continue;

			const auto& ANSIName = Compiler.get_name(Texture.id);
			auto Name = StringUtils::ANSIToString(ANSIName);

			auto Binding = Compiler.get_decoration(Texture.id, spv::DecorationBinding);

			MaterialProperty Property;
			Property.Type = MaterialPropertyType::Texture;
			Property.DataType = MaterialPropertyDataType::Undefined;
			Property.Width = 1;
			Property.Size = 0;
			Property.Offset = 0;
			Property.Binding = Binding;

			Properties[std::move(Name)] = Property;
		}
	}

	const MaterialProperty* ShaderReflection::FindProperty(const String& Name) const
	{
		auto Result = Properties.find(Name);
		if (Result == Properties.end())
			return nullptr;
		return &Result->second;
	}

	size_t ShaderReflection::GetTotalSizeForUniformBuffer() const
	{
		return SizeForUniformBuffer;
	}
}
