#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

layout(location = 0) in vec2 i_FragmentPosition;
layout(location = 1) flat in uint i_RectangleIndex;

layout(set = 0, binding = 0) buffer RectanglePrimitives
{
    RectanglePrimitive Array[];
} u_RectanglePrimitives;

layout(location = 0) out vec4 o_Color;

void main()
{
    RectanglePrimitive Rectangle = u_RectanglePrimitives.Array[i_RectangleIndex];

    o_Color = Rectangle.Color;
}
