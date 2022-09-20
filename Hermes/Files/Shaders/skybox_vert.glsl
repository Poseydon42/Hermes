#version 450
#pragma shader_stage(vertex)

#include "uniform_cube.glsl"

layout(push_constant, row_major) uniform PushConstantsStruct
{
    mat4 ViewProjection;
} PushConstants;

layout(location = 0) out vec3 o_Pos;

void main()
{
    vec3 LocalVertexPosition = UniformCubeVertices[gl_VertexIndex];
    vec4 ProjectedVertexPosition = PushConstants.ViewProjection * vec4(LocalVertexPosition, 1.0);

    o_Pos = LocalVertexPosition;

    // NOTE : set Z to 0 so that it is equal to the background value of the depth buffer
    ProjectedVertexPosition.z = 0;
    gl_Position = ProjectedVertexPosition;
}