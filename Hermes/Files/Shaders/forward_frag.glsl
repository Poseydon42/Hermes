#version 450
#pragma shader_stage(fragment)

#include "brdf_math.glsl"
#include "SharedData.h"

// NOTE : set 0 - global scene data, updated once for every frame

layout(set = 0, binding = 0, row_major) uniform GlobalSceneDataWrapper
{
    GlobalSceneData Data;
} u_SceneData;
layout(set = 0, binding = 1) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 2) uniform samplerCube u_SpecularMap;
layout(set = 0, binding = 3) uniform sampler2D u_PrecomputedBRDFMap;

// NOTE : set 1 - material data, updated for every material type

layout(set = 1, binding = 0) uniform sampler u_DefaultSampler;
layout(set = 1, binding = 1) uniform texture2D u_AlbedoTexture;
layout(set = 1, binding = 2) uniform texture2D u_RoughnessTexture;
layout(set = 1, binding = 3) uniform texture2D u_MetallicTexture;
layout(set = 1, binding = 4) uniform texture2D u_NormalTexture;

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) in vec3 i_FragmentPosition;
layout(location = 2) in vec3 i_FragmentNormal;
layout(location = 3) in mat3 i_TBNMatrix;

layout(location = 0) out vec4 o_Color;

vec4 CalculateLighting(vec3 Position, vec3 Normal, vec3 ViewVector)
{
    vec3 Result = vec3(0.0);

    vec3 AlbedoColor = texture(sampler2D(u_AlbedoTexture, u_DefaultSampler), i_TextureCoordinates).rgb;
    float Roughness = texture(sampler2D(u_RoughnessTexture, u_DefaultSampler), i_TextureCoordinates).r;
    float Metallic = texture(sampler2D(u_MetallicTexture, u_DefaultSampler), i_TextureCoordinates).r;

    for (int LightIndex = 0;  LightIndex < u_SceneData.Data.PointLightCount; LightIndex++)
    {
        PointLight Light = u_SceneData.Data.PointLights[LightIndex];

        vec3 LightDirection = normalize(Light.Position.xyz - Position);
        vec3 MedianVector = normalize(LightDirection + ViewVector);

        float Distance = length(Light.Position.xyz - Position);
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

    vec3 F = FresnelSchlickRoughness(AlbedoColor, Metallic, Roughness, max(dot(Normal, ViewVector), 0.0));

    vec3 SpecularCoef = F;
    vec3 DiffuseCoef = (1.0 - SpecularCoef) * (1.0 - Metallic);

    vec3 IrradianceValue = texture(u_IrradianceMap, Normal).rgb;
    vec3 DiffuseAmbient = IrradianceValue * AlbedoColor;

    uint NumberOfMipLevelsInSpecularEnvmap = textureQueryLevels(u_SpecularMap);
    vec3 SpecularMapValue = textureLod(u_SpecularMap, reflect(-ViewVector, Normal), Roughness * NumberOfMipLevelsInSpecularEnvmap).rgb;
    vec2 PrecomputedBRDFSampleCoordinates = vec2(max(dot(Normal, ViewVector), 0.0), Roughness);
    vec2 PrecomputedBRDFValue = texture(u_PrecomputedBRDFMap, PrecomputedBRDFSampleCoordinates).rg;
    vec3 SpecularAmbient = SpecularMapValue * (F * PrecomputedBRDFValue.x + PrecomputedBRDFValue.y);

    vec3 Ambient = DiffuseCoef * DiffuseAmbient + SpecularAmbient;
    Result += Ambient;

    return vec4(Result, 1.0);
}

void main()
{
    vec3 Normal = texture(sampler2D(u_NormalTexture, u_DefaultSampler), i_TextureCoordinates).rgb;
    Normal = Normal * 2.0 - 1.0; // Remapping into [-1;+1]
    Normal = normalize(i_TBNMatrix * Normal);

    vec3 Position = i_FragmentPosition;
    vec3 ViewVector = normalize(u_SceneData.Data.CameraLocation.xyz - Position);

    o_Color = CalculateLighting(Position, Normal, ViewVector);
}
