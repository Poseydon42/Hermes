#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 coord;
layout(location = 1) in vec2 tex_coord;

layout(push_constant, row_major) uniform PushConstant
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} PushConstants;

layout(location = 0) out vec2 passed_tex_coord;

vec4 ApplyDefaultNDCTransformations(vec4 Input)
{
    vec4 Multiplier = vec4(1.0, -1.0, 1.0, 1.0);
    return Input * Multiplier;
}

void main()
{
    vec4 Result = vec4(coord, 1.0);
    Result = PushConstants.Model * Result;
    Result = PushConstants.View * Result;
    Result = PushConstants.Projection * Result;
    Result = ApplyDefaultNDCTransformations(Result);
    gl_Position = Result;
    passed_tex_coord = tex_coord;
}
