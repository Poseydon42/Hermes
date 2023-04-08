#version 450
#pragma shader_stage(vertex)

#include "SharedData.h"
#include "fs_quad.glsl"

layout(location = 0) out vec2 o_FragmentLocation;

layout(push_constant) uniform PushConstants
{
    UIShaderPushConstants Constants;
} u_PushConstants;

void main()
{
    vec2 VertexLocation = FullScreenQuadVertices[gl_VertexIndex];

    o_FragmentLocation = VertexLocation * 0.5 + 0.5; // Transforming into [0;1] space
    gl_Position =  vec4(VertexLocation, 0.0, 1.0);
}
