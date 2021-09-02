#version 450
#pragma shader_stage(vertex)

vec3 vert_color[3] = 
{
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
};

layout(location = 0) in vec3 coord;

layout(location = 0) out vec3 vertex_color;

void main()
{
    gl_Position = vec4(coord, 1.0);
    vertex_color = vert_color[gl_VertexIndex % 3];
}
