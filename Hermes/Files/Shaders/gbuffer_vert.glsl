#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec2 i_TextureCoordinates;
layout(location = 2) in vec3 i_Normal;

layout(push_constant, row_major) uniform PushConstant
{
    mat4 Model;
} PerDrawcallData;

layout(set = 0, binding = 0, row_major) uniform PerFrameUBO
{
    mat4 ViewProjection;
} PerFrameData;

layout(location = 0) out vec2 o_TextureCoordinates;
layout(location = 1) out vec3 o_FragmentPosition;
layout(location = 2) out vec3 o_FragmentNormal;

void main()
{
    vec4 Result = PerFrameData.ViewProjection * PerDrawcallData.Model * vec4(i_Position, 1.0);
    gl_Position = Result;

    o_TextureCoordinates = i_TextureCoordinates;
    o_FragmentPosition = (PerDrawcallData.Model * vec4(i_Position, 1.0)).xyz;
    o_FragmentNormal = mat3(transpose(inverse(PerDrawcallData.Model))) * i_Normal;
}
