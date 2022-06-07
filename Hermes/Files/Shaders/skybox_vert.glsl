#version 450
#pragma shader_stage(vertex)

struct Vertex
{
    vec3 Position;
    vec2 TextureCoordinates;
} Vertices[6] = 
{
    // Right bottom - left bottom - right top
    { vec3(1.0, 1.0, 0.0), vec2(1.0, 0.0) },
    { vec3(-1.0, 1.0, 0.0), vec2(0.0, 0.0) },
    { vec3(1.0, -1.0, 0.0), vec2(1.0, 1.0) },
    
    //Left top - right top - left bottom
    { vec3(-1.0, -1.0, 0.0), vec2(0.0, 1.0) },
    { vec3(1.0, -1.0, 0.0), vec2(1.0, 1.0) },
    { vec3(-1.0, 1.0, 0.0), vec2(0.0, 0.0) }
};

layout(location = 0) out vec2 TextureCoordinates;

void main()
{
    gl_Position = vec4(Vertices[gl_VertexIndex].Position, 1.0);
    TextureCoordinates = Vertices[gl_VertexIndex].TextureCoordinates;
}