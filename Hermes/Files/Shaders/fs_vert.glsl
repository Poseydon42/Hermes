#version 450
#pragma shader_stage(vertex)

#include "fs_quad.glsl"

void main()
{
    gl_Position =  vec4(FullScreenQuadVertices[gl_VertexIndex], 0.0, 1.0);
}