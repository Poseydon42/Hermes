#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler AlbedoSampler;
layout(set = 1, binding = 1) uniform texture2D AlbedoTexture;

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 color;

void main()
{
    color = texture(sampler2D(AlbedoTexture, AlbedoSampler), tex_coord);
}
