#define PI 3.1415926535897932384626433832795

uint UnpackLightIndex(uint packedIndices, uint indexPosition)
{
	uint bitOffset = 16 * (indexPosition % 2);
	return (packedIndices >> bitOffset) & 0x0000ffff;
}

vec3 CalculateSpec(vec3 inViewDir, vec3 inLightDir, vec3 inNormal, vec3 inSpecColor, float inSpecStrength)
{
    // blinn-phong intermediate vector and spec value calculation
	vec3 intermediate = normalize(inViewDir + inLightDir);
	return inSpecColor * pow(max(dot(intermediate, inNormal), 0.0), 32) * inSpecStrength;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalculateLightInfluence(vec3 albedo, vec3 N, vec3 V, vec3 F, vec3 inPixelToLightDir, vec3 inLightColor, vec3 kD, float roughness)
{
		vec3 L = normalize(inPixelToLightDir);
        vec3 H = normalize(V + L);

        float attenuation = 1.0;
        vec3 radiance = inLightColor * attenuation;// * (1.0 - shadowAttenuation);

        float NDF = DistributionGGX(N, H, roughness);  
        float G = GeometrySmith(N, V, L, roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);
  
        float NdotL = max(dot(N, L), 0.0);        
        return (kD * albedo / PI + specular) * radiance * NdotL;
}

