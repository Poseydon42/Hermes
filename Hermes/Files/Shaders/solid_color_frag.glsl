#version 450
#pragma shader_stage(fragment)

#include "SharedData.h"

// NOTE : set 0 - global scene data, updated once for every frame

layout(set = 0, binding = 0, row_major) uniform GlobalSceneDataWrapper
{
    GlobalSceneData Data;
} u_SceneData;
layout(set = 0, binding = 1) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 2) uniform samplerCube u_SpecularMap;
layout(set = 0, binding = 3) uniform sampler2D u_PrecomputedBRDFMap;

// NOTE : set 1 - material data, updated for every material type

layout(set = 1, binding = 0) uniform MaterialData
{
    vec4 Color;
} u_MaterialData;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = u_MaterialData.Color;
}
