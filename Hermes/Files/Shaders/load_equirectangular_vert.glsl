#version 450
#pragma shader_stage(vertex)

layout(location = 0) out vec3 o_Pos;

layout(push_constant, row_major) uniform PushConstants
{
    mat4 ViewProjection;
} i_PushConstants;

vec3 Vertices[36] = {
    // back face
    { -1.0f, -1.0f, -1.0f }, // bottom-left
    {  1.0f,  1.0f, -1.0f }, // top-right
    {  1.0f, -1.0f, -1.0f }, // bottom-right         
    {  1.0f,  1.0f, -1.0f }, // top-right
    { -1.0f, -1.0f, -1.0f }, // bottom-left
    { -1.0f,  1.0f, -1.0f }, // top-left
    // front face
    { -1.0f, -1.0f,  1.0f }, // bottom-left
    {  1.0f, -1.0f,  1.0f }, // bottom-right
    {  1.0f,  1.0f,  1.0f }, // top-right
    {  1.0f,  1.0f,  1.0f }, // top-right
    { -1.0f,  1.0f,  1.0f }, // top-left
    { -1.0f, -1.0f,  1.0f }, // bottom-left
    // left face
    { -1.0f,  1.0f,  1.0f }, // top-right
    { -1.0f,  1.0f, -1.0f }, // top-left
    { -1.0f, -1.0f, -1.0f }, // bottom-left
    { -1.0f, -1.0f, -1.0f }, // bottom-left
    { -1.0f, -1.0f,  1.0f }, // bottom-right
    { -1.0f,  1.0f,  1.0f }, // top-right
    // right face
    {  1.0f,  1.0f,  1.0f }, // top-left
    {  1.0f, -1.0f, -1.0f }, // bottom-right
    {  1.0f,  1.0f, -1.0f }, // top-right         
    {  1.0f, -1.0f, -1.0f }, // bottom-right
    {  1.0f,  1.0f,  1.0f }, // top-left
    {  1.0f, -1.0f,  1.0f }, // bottom-left     
    // bottom face
    { -1.0f, -1.0f, -1.0f }, // top-right
    {  1.0f, -1.0f, -1.0f }, // top-left
    {  1.0f, -1.0f,  1.0f }, // bottom-left
    {  1.0f, -1.0f,  1.0f }, // bottom-left
    { -1.0f, -1.0f,  1.0f }, // bottom-right
    { -1.0f, -1.0f, -1.0f }, // top-right
    // top face
    { -1.0f,  1.0f, -1.0f }, // top-left
    {  1.0f,  1.0f , 1.0f }, // bottom-right
    {  1.0f,  1.0f, -1.0f }, // top-right     
    {  1.0f,  1.0f,  1.0f }, // bottom-right
    { -1.0f,  1.0f, -1.0f }, // top-left
    { -1.0f,  1.0f,  1.0f }  // bottom-left    
};

void main()
{
    vec3 LocalPos = Vertices[gl_VertexIndex];
    
    vec4 VertexPos = vec4(LocalPos, 1.0);
    VertexPos = i_PushConstants.ViewProjection * VertexPos;

    o_Pos = LocalPos;
    gl_Position = VertexPos;
}
