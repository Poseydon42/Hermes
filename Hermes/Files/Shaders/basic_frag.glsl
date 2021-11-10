#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable

// NOTE : keep in sync with engine's code
#define MAX_POINT_LIGHT_COUNT 256

layout(set = 1, binding = 0) uniform sampler AlbedoSampler;
layout(set = 1, binding = 1) uniform texture2D AlbedoTexture;

struct PointLight
{
    /* Only first 3 components are meaningful, 4th is added for alignment purposes */
    vec4 WorldPosition;
    vec4 Color;
    vec4 AttenuationCoefficients;
};

layout(set = 0, binding = 1) uniform LightingData
{
    PointLight PointLights[MAX_POINT_LIGHT_COUNT];
    uint PointLightCount;
    float AmbientLightingCoefficient;
} Lights;

layout(location = 0) in vec2 TextureCoordinates;
layout(location = 1) in vec3 FragmentPosition;
layout(location = 2) in vec3 FragmentNormal;

layout(location = 0) out vec4 OutputColor;

vec4 CalculateDiffuseLighting()
{
    vec3 Result = vec3(0.0);

    vec3 AlbedoColor = texture(sampler2D(AlbedoTexture, AlbedoSampler), TextureCoordinates).xyz;
    vec3 Normal = normalize(FragmentNormal);

    for (int LightIndex = 0;  LightIndex < Lights.PointLightCount; LightIndex++)
    {
        PointLight Light = Lights.PointLights[LightIndex];

        vec3 LightDirection = normalize(Light.WorldPosition.xyz - FragmentPosition);

        float AngleCoefficient = max(dot(Normal, LightDirection), 0.0);
        Result += AngleCoefficient * AlbedoColor;
    }
    
    Result += AlbedoColor * Lights.AmbientLightingCoefficient;
    return vec4(Result, 1.0);
}

void main()
{
    OutputColor = CalculateDiffuseLighting();
}
