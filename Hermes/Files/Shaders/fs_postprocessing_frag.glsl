#version 450
#pragma shader_stage(fragment)

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput i_Color;

layout(location = 0) in vec2 i_FragPos;

layout(location = 0) out vec4 o_Color;

void main()
{
    vec4 LoadedColor = subpassLoad(i_Color);

    // NOTE : simple Reinhard tone mapping
    vec3 RawColor = LoadedColor.rgb;
    vec3 MappedColor = RawColor / (RawColor + vec3(1.0));

    o_Color = vec4(MappedColor, LoadedColor.a);
}