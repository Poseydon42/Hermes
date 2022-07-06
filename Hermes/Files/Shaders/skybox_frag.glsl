#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform samplerCube Envmap;

layout(location = 0) in vec3 i_Pos;

layout(location = 0) out vec4 Color;

void main()
{
    Color = texture(Envmap, i_Pos);
}