#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(1.0, 0.8, 0.3, 1.0);
}
