#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec2 i_TextureCoordinates;

layout(set = 0, binding = 0) uniform sampler2D u_GlyphTexture;

layout(location = 0) out vec4 o_Color;

void main()
{
    float GlyphAlpha = texture(u_GlyphTexture, i_TextureCoordinates).r;
    
    const vec3 GlyphColor = vec3(1.0, 1.0, 1.0);
    o_Color = vec4(GlyphColor, GlyphAlpha);
}
