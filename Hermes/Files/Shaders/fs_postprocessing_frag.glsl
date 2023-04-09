#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform sampler2D u_Color;

layout(location = 0) in vec2 i_FragPos;

layout(location = 0) out vec4 o_Color;

void main()
{
    vec4 LoadedColor = texture(u_Color, i_FragPos);

    // NOTE : simple Reinhard tone mapping
    vec3 RawColor = LoadedColor.rgb;
    vec3 MappedColor = RawColor / (RawColor + vec3(1.0));

    o_Color = vec4(MappedColor, LoadedColor.a);
}