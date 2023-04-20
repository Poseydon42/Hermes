#version 450
#pragma shader_stage(compute)

#include "SharedData.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(set = 0, binding = 0, row_major) uniform GlobalSceneDataWrapper
{
    SceneData Data;
} u_SceneData;

layout(set = 0, binding = 1) buffer ClusterList
{
    uvec2 Clusters[]; // NOTE: x - index of first light, y - number of lights
} u_ClusterList;

layout(set = 0, binding = 2) coherent buffer LightIndexList
{
    uint IndexOfNextElement;
    uint Indices[];
} u_LightIndexList;

vec3 ScreenSpaceToViewSpace(vec2 FramebufferCoordinates)
{
    vec2 ScreenCoordinates = FramebufferCoordinates / u_SceneData.Data.ScreenDimensions;
    ScreenCoordinates.y = 1.0 - ScreenCoordinates.y;
    ScreenCoordinates = ScreenCoordinates * 2.0 - 1.0;
    vec4 ClipSpaceCoordinates = vec4(ScreenCoordinates, 1.0, 1.0); // Using reverse depth (near plane maps to 1)

    vec4 ViewSpaceCoordinates = u_SceneData.Data.InverseProjection * ClipSpaceCoordinates;
    ViewSpaceCoordinates /= ViewSpaceCoordinates.w;

    return ViewSpaceCoordinates.xyz;
}

vec3 IntersectCameraRayWithZPlane(vec3 RayDirection, float PlaneDistance)
{
    const vec3 Normal = vec3(0.0, 0.0, -1.0);

    vec3 Direction = normalize(RayDirection);
    
    float t = (PlaneDistance - dot(Normal, Direction)) / dot(Normal, Direction);

    vec3 Result = t * Direction;
    return Result;
}

bool TestSphereToAABB(vec3 MinCorner, vec3 MaxCorner, vec3 SpherePosition, float SphereRadius)
{
    if (SpherePosition.x + SphereRadius >= MinCorner.x && SpherePosition.x - SphereRadius <= MaxCorner.x &&
        SpherePosition.y + SphereRadius >= MinCorner.y && SpherePosition.y - SphereRadius <= MaxCorner.y &&
        SpherePosition.z + SphereRadius >= MinCorner.z && SpherePosition.z - SphereRadius <= MaxCorner.z)
    {
        return true;
    }

    return false;
}

shared uint s_LocalLightIndices[MAX_POINT_LIGHTS_IN_CLUSTER];
shared uint s_NextIndexInLocalLightIndexArray;

/*
 * TODO: this whole thing still needs quite a bit of work to be called a proper light culling shader
 * In particular:
 *   1. Check if we really need all the barriers that are present here
 *   2. We could probably use a better culling algorithm
 *   3. Using a better data structure for the culled light list would also be nice (at the moment there is a lot of duplicates and it requires 30+ MB for ~1000 lights)
 *   4. Many other things that can come up later
 * This shader also needs A TONN of optimization.
 */
void main()
{
    if (gl_WorkGroupID.x + gl_WorkGroupID.y + gl_WorkGroupID.z + gl_LocalInvocationID.x == 0)
        u_LightIndexList.IndexOfNextElement = 0;
    if (gl_LocalInvocationID.x == 0)
        s_NextIndexInLocalLightIndexArray = 0;
    memoryBarrier();

    // One workgroup per cluster, each thread withing a workgroup does the same calculations for now
    uint LinearTileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y;
    if (LinearTileIndex >= u_ClusterList.Clusters.length())
        return;

    float NearZ = u_SceneData.Data.CameraZBounds.x * pow(u_SceneData.Data.CameraZBounds.y / u_SceneData.Data.CameraZBounds.x, float(gl_WorkGroupID.z) / float(gl_NumWorkGroups.z));
    float FarZ  = u_SceneData.Data.CameraZBounds.x * pow(u_SceneData.Data.CameraZBounds.y / u_SceneData.Data.CameraZBounds.x, float(gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

    vec3 MinCornerOnNearCameraPlane = ScreenSpaceToViewSpace(vec2(min(u_SceneData.Data.MaxPixelsPerLightCluster * gl_WorkGroupID.xy, u_SceneData.Data.ScreenDimensions)));
    vec3 MaxCornerOnNearCameraPlane = ScreenSpaceToViewSpace(vec2(min(u_SceneData.Data.MaxPixelsPerLightCluster * (gl_WorkGroupID.xy + 1), u_SceneData.Data.ScreenDimensions)));

    vec3 MinPointNear = IntersectCameraRayWithZPlane(MinCornerOnNearCameraPlane, NearZ);
    vec3 MaxPointNear = IntersectCameraRayWithZPlane(MaxCornerOnNearCameraPlane, NearZ);
    vec3 MinPointFar = IntersectCameraRayWithZPlane(MinCornerOnNearCameraPlane, FarZ);
    vec3 MaxPointFar = IntersectCameraRayWithZPlane(MaxCornerOnNearCameraPlane, FarZ);

    vec3 MinCorner = min(min(MinPointNear, MinPointFar), min(MaxPointNear, MaxPointFar));
    vec3 MaxCorner = max(max(MinPointNear, MinPointFar), max(MaxPointNear, MaxPointFar));

    // This is where the workgroup splits - each individual thread processes a single light
    barrier();
    for (uint IndexOfFirstLightInBatch = 0; IndexOfFirstLightInBatch < u_SceneData.Data.PointLightCount; IndexOfFirstLightInBatch += gl_WorkGroupSize.x)
    {
        uint Index = IndexOfFirstLightInBatch + gl_LocalInvocationID.x;
        if (Index >= u_SceneData.Data.PointLightCount)
            continue;

        PointLight Light = u_SceneData.Data.PointLights[Index];

        const float LightIntensityTolerance = 0.01;

        // FIXME: optimize this (get rid of 2 square roots)
        vec3 Radiance = Light.Color.rgb * Light.Color.w;
        float Radius = sqrt(length(Radiance) / LightIntensityTolerance);

        vec3 LightPositionInViewSpace = (u_SceneData.Data.View * vec4(Light.Position.xyz, 1.0)).xyz;

        if (TestSphereToAABB(MinCorner, MaxCorner, LightPositionInViewSpace, Radius))
        {
            uint LocalIndex = atomicAdd(s_NextIndexInLocalLightIndexArray, 1);
            s_LocalLightIndices[LocalIndex] = Index;
            memoryBarrierShared();
        }
    }

    // Workgroup joins again and only thread #0 is copying the data to the global array
    // FIXME: can this be optimized?
    barrier();
    memoryBarrierShared(); // FIXME: I'm not too sure if we need memory barrier after execution barrier here
    if (gl_LocalInvocationID.x == 0)
    {
        uint NumberOfVisibleLights = s_NextIndexInLocalLightIndexArray;

        uint OffsetInGlobalList = atomicAdd(u_LightIndexList.IndexOfNextElement, NumberOfVisibleLights);

        for (uint Index = 0; Index < NumberOfVisibleLights; Index++)
            u_LightIndexList.Indices[OffsetInGlobalList + Index] = s_LocalLightIndices[Index];

        u_ClusterList.Clusters[LinearTileIndex].x = OffsetInGlobalList;
        u_ClusterList.Clusters[LinearTileIndex].y = NumberOfVisibleLights;
    }

    memoryBarrier(); // FIXME: again not too sure if a barrier is needed right at the end of the shader
}
