#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) flat in uint i_RectangleIndex;
layout(location = 2) in vec2 i_RelativeFragmentLocation; // NOTE: [-1, 1] range, origin in the center, +X goes to the right, +Y goes downwards

layout(set = 0, binding = 0) buffer RectanglePrimitives
{
    RectanglePrimitive Array[];
} u_RectanglePrimitives;

layout(set = 0, binding = 1) uniform sampler2D u_Textures[GMaxRectangleTextureCount];

layout(location = 0) out vec4 o_Color;

// NOTE: from https://iquilezles.org/articles/distfunctions/
float RectangleSDF(vec2 Location, vec2 HalfSize, float CornerRadius)
{
    vec2 A = abs(Location) - (HalfSize - CornerRadius);
    return length(max(A, 0.0)) + min(max(A.x, A.y), 0.0) - CornerRadius;
}

void main()
{
    RectanglePrimitive Rectangle = u_RectanglePrimitives.Array[i_RectangleIndex];

    vec4 TextureColor = texture(u_Textures[i_RectangleIndex], i_TextureCoordinates);

    float TextureWeight = Rectangle.TextureWeight;
    float ColorWeight = 1.0 - TextureWeight;

    float SDF = RectangleSDF(i_RelativeFragmentLocation * Rectangle.DimensionsInPixels / 2.0, Rectangle.DimensionsInPixels / 2.0, Rectangle.CornerRadius);
    vec4 OwnColor = Rectangle.OutlineColor * step(-Rectangle.OutlineRadius, SDF) + Rectangle.Color * (1.0 - step(-Rectangle.OutlineRadius, SDF));

    vec4 ComputedColor = TextureColor * TextureWeight + OwnColor * ColorWeight;
    if (SDF > 0.0)
        ComputedColor.a = 0.0;

    o_Color = ComputedColor;
}
