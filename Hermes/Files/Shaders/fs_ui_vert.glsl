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

    VertexLocation = VertexLocation * 0.5 + 0.5; // Transforming into [0;1] space
    o_FragmentLocation = VertexLocation;

    vec2 Scale = (u_PushConstants.Constants.TopRight - u_PushConstants.Constants.BottomLeft) * vec2(1.0, -1.0);
    vec2 Offset = vec2(u_PushConstants.Constants.BottomLeft.x, 1.0 - u_PushConstants.Constants.BottomLeft.y);

    VertexLocation = VertexLocation * Scale + Offset;

    VertexLocation = 2.0 * VertexLocation - 1.0; // And back into [-1;1]

    gl_Position =  vec4(VertexLocation, 0.0, 1.0);
}
