#version 450
#pragma shader_stage(vertex)

#include "fs_quad.glsl"

layout(location = 0) out vec2 o_FragPos;

void main()
{
    gl_Position =  vec4(FullScreenQuadVertices[gl_VertexIndex], 0.0, 1.0);
    o_FragPos = 0.5 * (FullScreenQuadVertices[gl_VertexIndex] + 1.0); // NOTE : transform from [-1;1] to [0;1]
}