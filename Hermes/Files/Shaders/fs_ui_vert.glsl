#version 450
#pragma shader_stage(vertex)

#include "SharedData.h"
#include "fs_quad.glsl"

layout(location = 0) in vec2 i_VertexPosition;

layout(location = 0) out vec2 o_FragmentLocation;
layout(location = 1) flat out uint o_RectangleIndex;

void main()
{
    o_FragmentLocation = i_VertexPosition * 0.5 + 0.5; // Transforming into [0;1] space
    o_RectangleIndex = gl_VertexIndex / 6;

    gl_Position =  vec4(i_VertexPosition, 0.0, 1.0);
}
