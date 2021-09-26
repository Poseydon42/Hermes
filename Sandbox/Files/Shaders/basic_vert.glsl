#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 coord;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec2 passed_tex_coord;

void main()
{
    gl_Position = vec4(coord, 1.0);
    passed_tex_coord = tex_coord;
}
