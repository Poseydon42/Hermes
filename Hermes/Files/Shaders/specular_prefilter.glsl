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

    float Roughness = u_MipLevelInfo.RoughnessLevel;

    vec3 Result = vec3(0.0);
    float TotalWeight = 0.0;
    for (uint SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
    {
        vec2 Xi = Hammersley(SampleIndex, SampleCount);
        vec3 H = ImportanceSampleGGX(Xi, N, Roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NDotL = dot(N, L);
        if (NDotL > 0.0)
        {
            float D = NormalDistribution(N, H, Roughness);
            float NDotH = max(dot(N, H), 0.0);
            float HDotV = max(dot(H, V), 0.0);
            float PDF = D * NDotH / (4.0 * HDotV) + 0.0001;

            vec2 Resolution = textureSize(u_Envmap, 0);
            float SATexel = 4.0 * Pi / (6.0 * Resolution.x * Resolution.y);
            float SASample = 1.0 / (float(SampleCount) * PDF);

            float MipLevel = Roughness == 0.0 ? 0.0 : 0.5 * log2(SASample / SATexel);

            Result += textureLod(u_Envmap, L, MipLevel).rgb * NDotL;
            TotalWeight += NDotL;
        }
    }
    Result /= TotalWeight;

    o_Color = vec4(Result, 1.0);
}
