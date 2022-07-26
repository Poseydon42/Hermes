#version 450
#pragma shader_stage(fragment)

//#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler u_DefaultSampler;
layout(set = 1, binding = 1) uniform texture2D u_AlbedoTexture;
layout(set = 1, binding = 2) uniform texture2D u_RoughnessTexture;
layout(set = 1, binding = 3) uniform texture2D u_MetallicTexture;
layout(set = 1, binding = 4) uniform texture2D u_NormalTexture;

layout(location = 0) in vec2 i_TextureCoordinates;
layout(location = 1) in vec3 i_FragmentPosition;
layout(location = 2) in vec3 i_FragmentNormal;
layout(location = 3) in mat3 i_TBNMatrix;

layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_PositionRoughness; // NOTE : 4-th component is roughness
layout(location = 2) out vec4 o_NormalMetallic; // NOTE : 4-th component is metallic

void main()
{
    vec4 AlbedoColor = texture(sampler2D(u_AlbedoTexture, u_DefaultSampler), i_TextureCoordinates);
    float Metallic = texture(sampler2D(u_MetallicTexture, u_DefaultSampler), i_TextureCoordinates).r;
    float Roughness = texture(sampler2D(u_RoughnessTexture, u_DefaultSampler), i_TextureCoordinates).r;
    vec3 Normal = texture(sampler2D(u_NormalTexture, u_DefaultSampler), i_TextureCoordinates).rgb;
    Normal = Normal * 2.0 - 1.0; // Remapping into [-1;+1]
    Normal = normalize(i_TBNMatrix * Normal);

    o_Albedo = AlbedoColor;
    o_PositionRoughness = vec4(i_FragmentPosition, Roughness);
    o_NormalMetallic = vec4(Normal, Metallic);
}
