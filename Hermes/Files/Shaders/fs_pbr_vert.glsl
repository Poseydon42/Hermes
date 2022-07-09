#version 450
#pragma shader_stage(vertex)

vec2 FullScreenQuadVertices[6] = 
{
    { -1.0, -1.0 },
    { -1.0,  1.0 },
    {  1.0,  1.0 },

    {  1.0,  1.0 },
    {  1.0, -1.0 },
    { -1.0, -1.0 }
};

void main()
{
    gl_Position =  vec4(FullScreenQuadVertices[gl_VertexIndex], 0.0, 1.0);
}