#include "ShaderReflection.h"

#include <spirv_cross.hpp>

#include "Core/Misc/StringUtils.h"
#include "Logging/Logger.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes
{
	static MaterialPropertyType SPIRVTypeToMaterialPropertyType(spirv_cross::SPIRType::BaseType Type)
	{
		switch (Type)
		{
		case spirv_cross::SPIRType::Float:
			return MaterialPropertyType::Float;
		default:
			HERMES_ASSERT_LOG(false, L"Unsupported SPIR-V type");
			return MaterialPropertyType::Undefined;
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

		const auto& TypeContainer = Compiler.get_type(MaterialDataUBO->base_type_id);
		SizeForUniformBuffer = Compiler.get_declared_struct_size(TypeContainer);
		for (uint32 MemberIndex = 0; MemberIndex < TypeContainer.member_types.size(); MemberIndex++)
		{
			const auto& ANSIName = Compiler.get_member_name(TypeContainer.self, MemberIndex);
			auto Name = StringUtils::ANSIToString(ANSIName);

			auto& NativeType = Compiler.get_type(TypeContainer.member_types[MemberIndex]);
			auto Type = SPIRVTypeToMaterialPropertyType(NativeType.basetype);

			auto Size = Compiler.get_declared_struct_member_size(TypeContainer, MemberIndex);
			auto Offset = Compiler.type_struct_member_offset(TypeContainer, MemberIndex);

			MaterialProperty Property;
			Property.Type = Type;
			Property.Width = NativeType.vecsize;
			Property.Size = Size;
			Property.Offset = Offset;

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
