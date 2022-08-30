#version 450
#pragma shader_stage(fragment)

#include "brdf_math.glsl"

// NOTE : keep in sync with engine's code
#define MAX_POINT_LIGHT_COUNT 256

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput i_Albedo;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput i_PositionRoughness;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput i_NormalMetallic;

layout(location = 0) in vec2 i_FragPos;

layout(location = 0) out vec4 o_Color;

struct PointLight
{
    /* Only first 3 components are meaningful, 4th is added for alignment purposes */
    vec4 WorldPosition;
    /* First 3 components are RGB color in range [0...1], 4th component is light power */
    vec4 Color;
    vec4 AttenuationCoefficients;
};

layout(set = 0, binding = 3) uniform LightingData
{
    PointLight PointLights[MAX_POINT_LIGHT_COUNT];
    vec4 CameraPosition;
    uint PointLightCount;
    float AmbientLightingCoefficient;
} u_Lights;

layout(set = 0, binding = 4) uniform samplerCube u_IrradianceMap;

vec4 CalculateLighting(vec3 Position, vec3 Normal, vec3 ViewVector)
{
    vec3 Result = vec3(0.0);

    vec3 AlbedoColor = subpassLoad(i_Albedo).rgb;
    float Roughness = subpassLoad(i_PositionRoughness).a;
    float Metallic = subpassLoad(i_NormalMetallic).a;

    for (int LightIndex = 0;  LightIndex < u_Lights.PointLightCount; LightIndex++)
    {
        PointLight Light = u_Lights.PointLights[LightIndex];

        vec3 LightDirection = normalize(Light.WorldPosition.xyz - Position);
        vec3 MedianVector = normalize(LightDirection + ViewVector);

        float Distance = length(Light.WorldPosition.xyz - Position);
        float Attenuation = 1.0 / (Distance * Distance);
        vec3 Radiance = Light.Color.rgb * Attenuation * Light.Color.w;
        float AngleCoefficient = max(dot(Normal, LightDirection), 0.0);

        float NDF = NormalDistribution(Normal, MedianVector, Roughness);
        float G = GeometrySmithFunction(Normal, ViewVector, LightDirection, Roughness);
        vec3 F = FresnelSchlick(AlbedoColor, Metallic, max(dot(MedianVector, ViewVector), 0.0));

        vec3 Numerator = NDF * G * F;
        float Denominator = 4.0 * max(dot(Normal, ViewVector), 0.0) * max(dot(Normal, LightDirection), 0.0) + 0.00001;
        vec3 Specular = Numerator / Denominator;

        vec3 DiffuseCoef = vec3(1.0) - F;
        DiffuseCoef *= 1.0 - Metallic;

        Result += (DiffuseCoef * AlbedoColor / Pi + Specular) * Radiance * max(dot(Normal, LightDirection), 0.0);
    }

    vec3 IrradianceValue = texture(u_IrradianceMap, Normal).rgb;
    vec3 DiffuseCoef = 1.0 - FresnelSchlickRoughness(AlbedoColor, Metallic, Roughness, max(dot(Normal, ViewVector), 0.0));
    vec3 Ambient = DiffuseCoef * IrradianceValue * AlbedoColor;
    Result += Ambient;

    return vec4(Result, 1.0);
}

void main()
{
    vec3 Position = subpassLoad(i_PositionRoughness).xyz;
    vec3 Normal = subpassLoad(i_NormalMetallic).xyz;
    vec3 ViewVector = normalize(u_Lights.CameraPosition.xyz - Position);

    o_Color = CalculateLighting(Position, Normal, ViewVector);
}
