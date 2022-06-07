#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform CameraData
{
    float Pitch, Yaw, VerticalFOV, HorizontalFOV;
} Camera;

layout(set = 0, binding = 1) uniform sampler Envmap;

layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput DepthBuffer;

layout(location = 0) in vec2 TextureCoordinates;

layout(location = 0) out vec4 Color;

void main()
{
    if (subpassLoad(DepthBuffer).r >= 0.9999999)
        Color = vec4(TextureCoordinates, 0.4, 1.0);
}