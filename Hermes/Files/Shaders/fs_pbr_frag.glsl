#version 450
#pragma shader_stage(fragment)

// NOTE : keep in sync with engine's code
#define MAX_POINT_LIGHT_COUNT 256
#define Pi 3.14159265359

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput i_Albedo;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput i_PositionRoughness;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput i_NormalMetallic;

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

float NormalDistribution(vec3 Normal, vec3 MedianVector, float Roughness)
{
    float CosAngle = max(dot(Normal, MedianVector), 0.0);

    float Nominator = pow(Roughness, 4);

    float Denominator = (CosAngle * CosAngle * (pow(Roughness, 4) - 1.0) + 1.0);
    Denominator = Pi * Denominator * Denominator;

    return Nominator / Denominator;
}

float GeometrySchlickGGX(float CosAngle, float Roughness)
{
    float k = (Roughness + 1) * (Roughness + 1) / 8.0; // TODO : are these values correct?

    float Nominator = CosAngle;
    float Denominator = CosAngle * (1 - k) + k;

    return Nominator / Denominator;
}

float GeometrySmithFunction(vec3 Normal, vec3 View, vec3 Light, float Roughness)
{
    float CosAngleNormalView = max(dot(Normal, View), 0.0);
    float CosAngleNormalLight = max(dot(Normal, Light), 0.0);

    float GGXView = GeometrySchlickGGX(CosAngleNormalView, Roughness);
    float GGXLight = GeometrySchlickGGX(CosAngleNormalLight, Roughness);

    return GGXView * GGXLight;
}

vec3 FresnelSchlick(vec3 AlbedoColor, float Metalness, float CosAngle)
{
    vec3 F0 = mix(vec3(0.04), AlbedoColor, Metalness);
    return F0 + (1.0 - F0) * pow(1.0 - CosAngle, 5.0);
}

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

    return vec4(Result, 1.0);
}

void main()
{
    vec3 Position = subpassLoad(i_PositionRoughness).xyz;
    vec3 Normal = subpassLoad(i_NormalMetallic).xyz;
    vec3 ViewVector = normalize(u_Lights.CameraPosition.xyz - Position);

    o_Color = CalculateLighting(Position, Normal, ViewVector);
}
