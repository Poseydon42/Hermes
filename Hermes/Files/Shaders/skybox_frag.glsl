#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0, row_major) uniform CameraData
{
    mat4 ViewMatrix;
    vec2 ViewportDimensions;
    float HalfVertifalFOV;
    float AspectRatio;
} Camera;

layout(set = 0, binding = 1) uniform sampler2D Envmap;

layout(location = 0) in vec2 TextureCoordinates;

layout(location = 0) out vec4 Color;

const vec2 InverseAtan = vec2(0.1591, 0.3183);

void main()
{
    float Dist = 1.0 / tan(Camera.HalfVertifalFOV);
    vec2 FragCoords = (gl_FragCoord.xy / Camera.ViewportDimensions - vec2(0.5)) * 2.0;
    vec3 Direction = normalize(inverse(Camera.ViewMatrix) * vec4(-FragCoords.x / Camera.AspectRatio, FragCoords.y, Dist, 0.0)).xyz;

    vec2 UV = vec2(atan(Direction.z, Direction.x), asin(Direction.y));
    UV *= InverseAtan;
    UV += 0.5;

    Color = vec4(texture(Envmap, UV).rgb, 1.0);
}