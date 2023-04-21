#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec2 i_VertexPosition;
layout(location = 1) in vec2 i_TextureCoordinates;

layout(location = 0) out vec2 o_TextureCoordinates;

void main()
{
    o_TextureCoordinates = i_TextureCoordinates;
    gl_Position = vec4(i_VertexPosition, 0.0, 1.0);
}
