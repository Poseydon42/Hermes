#version 450
#pragma shader_stage(vertex)

#include "SharedData.h"

layout(location = 0) in vec2 i_VertexPosition;
layout(location = 1) in vec2 i_TextureCoordinates;
layout(location = 2) in vec2 i_VertexLocationInRectangle;

layout(location = 0) out vec2 o_TextureCoordinates;
layout(location = 1) flat out uint o_RectangleIndex;
layout(location = 2) out vec2 o_RelativeFragmentLocation; // NOTE: [-1, 1] range, origin in the center, +X goes to the right, +Y goes downwards

void main()
{
    o_TextureCoordinates = i_TextureCoordinates;
    o_RectangleIndex = gl_VertexIndex / 6;
    o_RelativeFragmentLocation = i_VertexLocationInRectangle;

    gl_Position =  vec4(i_VertexPosition, 0.0, 1.0);
}
