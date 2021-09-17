#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 coord;

void main()
{
    gl_Position = vec4(coord, 1.0);
}
