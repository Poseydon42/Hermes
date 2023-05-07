#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) flat in uint i_RectangleIndex;

layout(set = 0, binding = 0) buffer RectanglePrimitives
{
    RectanglePrimitive Array[];
} u_RectanglePrimitives;

layout(set = 0, binding = 1) uniform sampler2D u_Textures[GMaxRectangleTextureCount];

layout(location = 0) out vec4 o_Color;

void main()
{
    RectanglePrimitive Rectangle = u_RectanglePrimitives.Array[i_RectangleIndex];

    vec4 TextureColor = texture(u_Textures[i_RectangleIndex], i_TextureCoordinates);
    vec4 OwnColor = Rectangle.Color;

    float TextureWeight = Rectangle.TextureWeight;
    float ColorWeight = 1.0 - TextureWeight;

    o_Color = TextureColor * TextureWeight + OwnColor * ColorWeight;
}
