const float Pi = 3.14159265359;

float NormalDistribution(vec3 Normal, vec3 MedianVector, float Roughness)
{
    float CosAngle = max(dot(Normal, MedianVector), 0.0);

    float Nominator = pow(Roughness, 4);

    float Denominator = (CosAngle * CosAngle * (pow(Roughness, 4) - 1.0) + 1.0);
    Denominator = Pi * Denominator * Denominator;

    return Nominator / Denominator;
}

float GeometrySchlickGGX(float CosAngle, float Roughness)
{
    float k = Roughness * Roughness / 2.0; // TODO : are these values correct?

    float Nominator = CosAngle;
    float Denominator = CosAngle * (1 - k) + k;

    return Nominator / Denominator;
}

float GeometrySmithFunction(vec3 Normal, vec3 View, vec3 Light, float Roughness)
{
    float CosAngleNormalView = max(dot(Normal, View), 0.0);
    float CosAngleNormalLight = max(dot(Normal, Light), 0.0);

    float GGXView = GeometrySchlickGGX(CosAngleNormalView, Roughness);
    float GGXLight = GeometrySchlickGGX(CosAngleNormalLight, Roughness);

    return GGXView * GGXLight;
}

vec3 ComputeF0(vec3 AlbedoColor, float Metalness)
{
    return mix(vec3(0.04), AlbedoColor, Metalness);
}

vec3 FresnelSchlick(vec3 AlbedoColor, float Metalness, float CosAngle)
{
    vec3 F0 = ComputeF0(AlbedoColor, Metalness);
    return F0 + (1.0 - F0) * pow(1.0 - CosAngle, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 AlbedoColor, float Metalness, float Roughness, float CosAngle)
{
    vec3 F0 = ComputeF0(AlbedoColor, Metalness);
    return F0 + (max(vec3(1.0 - Roughness), F0) - F0) * pow(clamp(1.0 - CosAngle, 0.0, 1.0), 5.0);
}

float RadicalInverse_VdC(uint Bits) 
{
    Bits = (Bits << 16u) | (Bits >> 16u);
    Bits = ((Bits & 0x55555555u) << 1u) | ((Bits & 0xAAAAAAAAu) >> 1u);
    Bits = ((Bits & 0x33333333u) << 2u) | ((Bits & 0xCCCCCCCCu) >> 2u);
    Bits = ((Bits & 0x0F0F0F0Fu) << 4u) | ((Bits & 0xF0F0F0F0u) >> 4u);
    Bits = ((Bits & 0x00FF00FFu) << 8u) | ((Bits & 0xFF00FF00u) >> 8u);
    return float(Bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint I, uint N)
{
    return vec2(float(I)/float(N), RadicalInverse_VdC(I));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 Normal, float Roughness)
{
    float A = Roughness * Roughness;

    float Phi = 2.0 * Pi * Xi.x;
    float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (A * A - 1.0) * Xi.y));
    float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

    vec3 H = vec3(cos(Phi) * SinTheta, sin(Phi) * SinTheta, CosTheta);

    // Tangent space sample vector to world space sample vector
    vec3 Up = abs(Normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 Tangent = normalize(cross(Up, Normal));
    vec3 Bitangent = cross(Normal, Tangent);

    vec3 Result = Tangent * H.x + Bitangent * H.y + Normal * H.z;
    return normalize(Result);
}
