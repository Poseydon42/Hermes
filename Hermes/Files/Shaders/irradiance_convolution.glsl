#version 450
#pragma shader_stage(fragment)

layout(set = 0, binding = 0) uniform samplerCube u_Envmap;

layout(location = 0) in vec3 i_Position;

layout(location = 0) out vec4 o_Color;

const float Pi = 3.14159265359;

/* https://learnopengl.com/PBR/IBL/Diffuse-irradiance */
void main()
{
    vec3 Normal = normalize(i_Position);

    vec3 Result = vec3(0.0);

    vec3 Up = vec3(0.0, 1.0, 0.0);
    if (abs(Normal.y) > 0.999)
    {
        Up = vec3(0.0, 0.0, 1.0);
    }
    vec3 Right = normalize(cross(Up, Normal));
    Up = normalize(cross(Normal, Right));

    float SampleDelta = 0.01;
    float SampleCount = 0.0;
    for (float Phi = 0.0; Phi < 2.0 * Pi; Phi += SampleDelta)
    {
        for (float Theta = 0.0; Theta < 0.5 * Pi; Theta += SampleDelta)
        {
            // Spherical to cartesian in tangent space
            vec3 TangentSampleDirection = vec3(sin(Theta) * cos(Phi), sin(Theta) * sin(Phi), cos(Theta));
            // Tangent sample direction to world sample direction
            vec3 WorldSampleDirection = TangentSampleDirection.x * Right + TangentSampleDirection.y * Up + TangentSampleDirection.z * Normal;

            Result += texture(u_Envmap, WorldSampleDirection).rgb * cos(Theta) * sin(Theta);
            SampleCount += 1.0;
        }
    }

    Result /= SampleCount;

    o_Color = vec4(Result, 1.0);
}
