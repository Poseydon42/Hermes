#version 450
#pragma shader_stage(fragment)

#define Pi 3.14159265359

layout(location = 0) in vec3 i_Pos;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform sampler2D d_EquirectangularTexture;

vec2 UVOnEquirectangularProjection(vec3 Dir)
{
    float Theta = acos(Dir.y);
    float Phi = atan(Dir.z, Dir.x) + Pi;

    vec2 Result = vec2(Phi / (2 * Pi), Theta / Pi);
    return Result;
}

void main()
{
    vec2 UV = UVOnEquirectangularProjection(normalize(i_Pos));
    vec4 Color = texture(d_EquirectangularTexture, UV);

    o_Color = Color;
}
