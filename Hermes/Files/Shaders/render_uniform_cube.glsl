#version 450
#pragma shader_stage(vertex)

#include "uniform_cube.glsl"

layout(location = 0) out vec3 o_Pos;

layout(push_constant, row_major) uniform PushConstants
{
    mat4 ViewProjection;
} i_PushConstants;


void main()
{
    vec3 LocalPos = UniformCubeVertices[gl_VertexIndex];
    
    vec4 VertexPos = vec4(LocalPos, 1.0);
    VertexPos = i_PushConstants.ViewProjection * VertexPos;

    o_Pos = LocalPos;
    gl_Position = VertexPos;
}
