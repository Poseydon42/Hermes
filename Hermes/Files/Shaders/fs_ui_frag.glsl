#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

layout(location = 0) in vec2 i_FragmentPosition;

layout(push_constant) uniform PushConstants
{
    UIShaderPushConstants Constants;
} u_PushConstants;

layout(set = 0, binding = 0) buffer RectanglePrimitives
{
    RectanglePrimitive Array[];
} u_RectanglePrimitives;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(0.0, 0.0, 0.0, 0.0);

    for (uint RectangleIndex = u_PushConstants.Constants.FirstRectangle; RectangleIndex < u_PushConstants.Constants.RectangleCount; RectangleIndex++)
    {
        RectanglePrimitive Rectangle = u_RectanglePrimitives.Array[RectangleIndex];

        if (i_FragmentPosition.x >= Rectangle.Min.x && i_FragmentPosition.x < Rectangle.Max.x &&
            i_FragmentPosition.y >= Rectangle.Min.y && i_FragmentPosition.y < Rectangle.Max.y)
        {
            o_Color = vec4(Rectangle.Color, 1.0);
        }
    }
}
