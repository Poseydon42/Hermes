#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureCoordinates;
layout(location = 2) in vec3 Normal;

layout(push_constant, row_major) uniform PushConstant
{
    mat4 Model;
} PerDrawcallData;

layout(set = 0, binding = 0, row_major) uniform PerFrameUBO
{
    mat4 ViewProjection;
} PerFrameData;

layout(location = 0) out vec2 PassedTextureCoordinates;
layout(location = 1) out vec3 PassedFragmentPosition;
layout(location = 2) out vec3 PassedFragmentNormal;


void main()
{
    vec4 Result = PerFrameData.ViewProjection * PerDrawcallData.Model * vec4(Position, 1.0);
    gl_Position = Result;

    PassedTextureCoordinates = TextureCoordinates;
    PassedFragmentPosition = (PerDrawcallData.Model * vec4(Position, 1.0)).xyz;
    PassedFragmentNormal = mat3(transpose(inverse(PerDrawcallData.Model))) * Normal;
}
