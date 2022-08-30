#version 450
#pragma shader_stage(fragment)

#include "brdf_math.glsl"

layout(location = 0) in vec2 i_FragPos;

layout(location = 0) out vec4 o_Color;

/* https://learnopengl.com/PBR/IBL/Specular-IBL */
void main()
{
    float NDotV = i_FragPos.x;
    float Roughness = i_FragPos.y;

    vec3 V = vec3(sqrt(1.0 - NDotV * NDotV), 0.0, NDotV);

    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SampleCount = 1024;
    for (uint SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
    {
        vec2 Xi = Hammersley(SampleIndex, SampleCount);
        vec3 H = ImportanceSampleGGX(Xi, N, Roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NDotL = max(L.z, 0.0);
        float NDotH = max(H.z, 0.0);
        float VDotH = max(dot(V, H), 0.0);

        if (NDotL > 0.0)
        {
            float G = GeometrySmithFunction(N, V, L, Roughness);
            float GVis = (G * VDotH) / (NDotH * NDotV);
            float F = pow(1.0 - VDotH, 5.0);

            A += (1.0 - F) * GVis;
            B += F * GVis;
        }
    }

    A /= float(SampleCount);
    B /= float(SampleCount);

    o_Color = vec4(A, B, 0.0, 1.0);
}