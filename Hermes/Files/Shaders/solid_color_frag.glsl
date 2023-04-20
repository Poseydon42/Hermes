#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

layout(set = 0, binding = 0) uniform GlobalSceneDataWrapper
{
    SceneData Data;
} u_GlobalSceneDataWrapper;

layout(set = 1, binding = 0) uniform MaterialData
{
    vec3 Color;
} u_MaterialData;

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) in vec3 i_FragmentPosition;
layout(location = 2) in vec3 i_FragmentNormal;
layout(location = 3) in mat3 i_TBNMatrix;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(u_MaterialData.Color, 1.0);
}
