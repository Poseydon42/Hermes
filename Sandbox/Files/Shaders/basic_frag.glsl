#version 450
#pragma shader_stage(fragment)

layout(binding = 0) uniform UniformBuffer
{
    vec3 Color;
} UBO;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(UBO.Color, 1.0);
}