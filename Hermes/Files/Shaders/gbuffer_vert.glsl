#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec2 i_TextureCoordinates;
layout(location = 2) in vec3 i_Normal;
layout(location = 3) in vec3 i_Tangent;

layout(push_constant, row_major) uniform PushConstant
{
    mat4 Model;
} PerDrawcallData;

layout(set = 0, binding = 0, row_major) uniform PerFrameUBO
{
    mat4 ViewProjection;
} PerFrameData;

layout(location = 0) out vec2 o_TextureCoordinates;
layout(location = 1) out vec3 o_FragmentPosition;
layout(location = 2) out vec3 o_FragmentNormal;
layout(location = 3) out mat3 o_TBNMatrix;

void main()
{
    vec4 Result = PerFrameData.ViewProjection * PerDrawcallData.Model * vec4(i_Position, 1.0);
    gl_Position = Result;

    mat3 NormalMatrix = mat3(transpose(inverse(PerDrawcallData.Model)));
    vec3 Normal = normalize(NormalMatrix * i_Normal);

    o_TextureCoordinates = i_TextureCoordinates;
    o_FragmentPosition = (PerDrawcallData.Model * vec4(i_Position, 1.0)).xyz;
    o_FragmentNormal = Normal;

    vec3 Tangent = normalize(NormalMatrix * i_Tangent);
    // Reorthogonalize tangent
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = normalize(cross(Normal, Tangent));
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    o_TBNMatrix = TBN;
}
