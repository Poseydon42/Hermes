#version 450
#pragma shader_stage(vertex)

#include "SharedData.h"

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec2 i_TextureCoordinates;
layout(location = 2) in vec3 i_Normal;
layout(location = 3) in vec3 i_Tangent;

layout(push_constant, row_major) uniform GlobalDrawcallDataWrapper
{
    GlobalDrawcallData Data;
} u_DrawcallData;

layout(set = 0, binding = 0, row_major) uniform GlobalSceneDataWrapper
{
    GlobalSceneData Data;
} u_SceneData;

layout(location = 0) out vec2 o_TextureCoordinates;

void main()
{
    gl_Position = u_SceneData.Data.ViewProjection * u_DrawcallData.Data.ModelMatrix * vec4(i_Position, 1.0);
    o_TextureCoordinates = i_TextureCoordinates;
}
