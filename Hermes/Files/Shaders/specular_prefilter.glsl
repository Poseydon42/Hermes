#version 450
#pragma shader_stage(fragment)

#include "brdf_math.glsl"

layout(set = 0, binding = 0) uniform samplerCube u_Envmap;
layout(push_constant) uniform MipLevelInfo
{
    layout(offset = 64) float RoughnessLevel; // skip MVP matrix in the vertex shader
} u_MipLevelInfo;

layout(location = 0) in vec3 i_Position;

layout(location = 0) out vec4 o_Color;

const uint SampleCount = 4096;

/* https://learnopengl.com/PBR/IBL/Specular-IBL */
void main()
{
    vec3 N = normalize(i_Position);

    vec3 V = N;

    vec3 Result = vec3(0.0);
    float TotalWeight = 0.0;
    for (uint SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
    {
        vec2 Xi = Hammersley(SampleIndex, SampleCount);
        vec3 H = ImportanceSampleGGX(Xi, N, u_MipLevelInfo.RoughnessLevel);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NDotL = dot(N, L);
        if (NDotL > 0.0)
        {
            Result += texture(u_Envmap, L).rgb * NDotL;
            TotalWeight += NDotL;
        }
    }
    Result /= TotalWeight;

    o_Color = vec4(Result, 1.0);
}
