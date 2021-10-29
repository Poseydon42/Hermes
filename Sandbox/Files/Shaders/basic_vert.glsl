#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 coord;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;

layout(push_constant, row_major) uniform PushConstant
{
    mat4 Model;
} PerDrawcallData;

layout(set = 0, binding = 0, row_major) uniform PerFrameUBO
{
    mat4 ViewProjection;
} PerFrameData;

layout(location = 0) out vec2 passed_tex_coord;

void main()
{
    vec4 Result = PerFrameData.ViewProjection * PerDrawcallData.Model * vec4(coord, 1.0);
    gl_Position = Result;
    passed_tex_coord = tex_coord;
}
