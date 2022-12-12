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
layout(set = 0, binding = 4) buffer ClusterList
{
    uvec2 Clusters[]; // NOTE: x - index of first light, y - number of lights
} u_ClusterList;
layout(set = 0, binding = 5) buffer LightIndexList
{
    uint IndexOfNextElement;
    uint Indices[];
} u_LightIndexList;


// NOTE : set 1 - material data, updated for every material type; we can't use binding #0 because it can only be used for uniform buffer containing numeric properties

layout(set = 1, binding = 1) uniform sampler2D u_AlbedoTexture;
layout(set = 1, binding = 2) uniform sampler2D u_RoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D u_MetallicTexture;
layout(set = 1, binding = 4) uniform sampler2D u_NormalTexture;

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) in vec3 i_FragmentPosition;
layout(location = 2) in vec3 i_FragmentNormal;
layout(location = 3) in mat3 i_TBNMatrix;

layout(location = 0) out vec4 o_Color;

float LinearDepth(float SampledDepth)
{
    float NearZ = u_SceneData.Data.CameraZBounds.x;
    float FarZ  = u_SceneData.Data.CameraZBounds.y;

    float Numerator = -NearZ * FarZ;
    float Denominator = SampledDepth * (FarZ - NearZ) + NearZ;

    return Numerator / Denominator;
}

vec4 CalculateLighting(vec3 Position, vec3 Normal, vec3 ViewVector)
{
    vec3 Result = vec3(0.0);

    vec3 AlbedoColor = texture(u_AlbedoTexture, i_TextureCoordinates).rgb;
    float Roughness = texture(u_RoughnessTexture, i_TextureCoordinates).r;
    float Metallic = texture(u_MetallicTexture, i_TextureCoordinates).r;

    float NearZ = u_SceneData.Data.CameraZBounds.x;
    float FarZ  = u_SceneData.Data.CameraZBounds.y;

    // NOTE: we use negative here because even though depth in view space has negative values we need to use the positive value for indexing the cluster
    float Depth = -LinearDepth(gl_FragCoord.z);

    // FIXME: optimize this - coefficients can be calculated once on the CPU side
    uint ClusterZIndex = clamp(uint(floor(u_SceneData.Data.NumberOfZClusters.x * log(Depth) / log(FarZ / NearZ) - u_SceneData.Data.NumberOfZClusters.x * log(NearZ) / log(FarZ / NearZ))), 0, u_SceneData.Data.NumberOfZClusters.x - 1);
    uvec2 ClusterXYIndex = uvec2((gl_FragCoord.xy - 0.5) / u_SceneData.Data.MaxPixelsPerLightCluster);

    uvec2 NumberOfXYClusters = uvec2(ceil(u_SceneData.Data.ScreenDimensions / u_SceneData.Data.MaxPixelsPerLightCluster));

    uint LightClusterIndex = ClusterXYIndex.x + ClusterXYIndex.y * NumberOfXYClusters.x + ClusterZIndex * NumberOfXYClusters.x * NumberOfXYClusters.y;
    if (LightClusterIndex >= u_ClusterList.Clusters.length())
        discard;

#define USE_CLUSTERED_LIGHTING 1
#if defined(USE_CLUSTERED_LIGHTING) && USE_CLUSTERED_LIGHTING
    uint FirstLight = u_ClusterList.Clusters[LightClusterIndex].x;
    uint OnePastLastLight  = FirstLight + u_ClusterList.Clusters[LightClusterIndex].y;
    for (uint Index = FirstLight; Index < OnePastLastLight; Index++)
    {
        uint LightIndex = u_LightIndexList.Indices[Index];
#else
    for (uint LightIndex = 0; LightIndex < u_SceneData.Data.PointLightCount; LightIndex++)
    {
#endif
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
    vec3 Normal = texture(u_NormalTexture, i_TextureCoordinates).rgb;
    Normal = Normal * 2.0 - 1.0; // Remapping into [-1;+1]
    Normal = normalize(i_TBNMatrix * Normal);

    vec3 Position = i_FragmentPosition;
    vec3 ViewVector = normalize(u_SceneData.Data.CameraLocation.xyz - Position);

    o_Color = CalculateLighting(Position, Normal, ViewVector);
}
