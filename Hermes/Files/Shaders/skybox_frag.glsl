#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform CameraData
{
    float Pitch, Yaw, VerticalFOV, HorizontalFOV;
} Camera;

layout(set = 0, binding = 1) uniform sampler Envmap;

layout(location = 0) in vec2 TextureCoordinates;

layout(location = 0) out vec4 Color;

void main()
{
    Color = vec4(0.5, 0.5, 0.5, 1.0);
}