const uint MaxPointLightCount = 256;

struct PointLight
{
    /* Only first 3 components are meaningful, 4th is added for alignment purposes */
    vec4 Position;
    vec4 Color;
};

struct GlobalDrawcallData
{
    mat4 ModelMatrix;
};

struct GlobalSceneData
{
    mat4 ViewProjection;
    vec4 CameraLocation;
    PointLight PointLights[MaxPointLightCount];
    uint PointLightCount;
};
