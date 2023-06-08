#version 460

out vec4 FragColor;


layout(binding = 0) uniform sampler2D U_ALBEDO_MAP;
layout(binding = 1) uniform sampler2D U_NORMAL_MAP;
layout(binding = 2) uniform sampler2D U_METALLIC_ROUGHNESS_MAP;
layout(binding = 3) uniform sampler2D U_OCCLUSION_MAP;
layout(binding = 4) uniform sampler2D U_EMISSIVE_MAP;

layout(binding = 5) uniform sampler2D U_SHADOW_MAP;

struct DirectionalLight {
    vec3 position;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float distance;
    float decay;
};

uniform DirectionalLight U_DIRECTIONAL_LIGHT;

uniform PointLight U_POINT_LIGHT;




const float PI = 3.14159265359;

uniform vec3 U_CAMERA_POS;

float distribution_ggx(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 calc_bump_normal(vec3 normal, vec3 tangent, vec3 bitangent, vec3 normal_map)
{
    mat3 tbn = mat3(tangent, bitangent, normal);
    return normalize(tbn * normal_map);
}

vec3 srgb_to_linear(vec3 color) {
    return pow(color, vec3(2.2));
}

vec3 linear_to_srgb(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}




in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
    vec3 bitangent;
    vec4 fragPosLightSpace;
} vs_out;

float shadowCalculation(vec4 fragPosLightSpace, float bias) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(U_SHADOW_MAP, projCoords.xy).r; 
    float currentDepth = projCoords.z; 
    
    if (currentDepth > 1.0) {
        return 0.0;
    }
    
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  


    return shadow;   
}



void main()
{
    vec3 albedo = texture(U_ALBEDO_MAP, vs_out.uv).rgb;
    // vec3 normal = texture(U_NORMAL_MAP, vs_out.uv).rgb;
    // normal = normal * 2.0 - 1.0;
    // normal = calc_bump_normal(vs_out.normal, vs_out.tangent, vs_out.bitangent, normal);
    vec3 normal = normalize(vs_out.normal);
    vec3 directional_light_dir = normalize(U_DIRECTIONAL_LIGHT.position - vs_out.position);
    vec3 directional_light = U_DIRECTIONAL_LIGHT.color * U_DIRECTIONAL_LIGHT.intensity * max(dot(normal, directional_light_dir), 0.0);

    vec3 diffuse = albedo * directional_light;
    
    float bias = max(0.05 * (1.0 - dot(normal, directional_light_dir)), 0.005);  

    float shadow = shadowCalculation(vs_out.fragPosLightSpace, bias);

    vec3 finalColor = diffuse * (1.0 - shadow);

    // vec3 ambient = vec3(0.01) * albedo * U_DIRECTIONAL_LIGHT.color * U_DIRECTIONAL_LIGHT.intensity;

    //    float metallic = texture(U_METALLIC_ROUGHNESS_MAP, vs_out.uv).b;
    //    float roughness = texture(U_METALLIC_ROUGHNESS_MAP, vs_out.uv).g;
    //
    //    vec3 emissive = texture(U_EMISSIVE_MAP, vs_out.uv).rgb;
    //
    //    vec3 N = normalize(normal);
    //
    //    vec3 V = normalize(U_CAMERA_POS - vs_out.position);
    //
    //    vec3 F0 = vec3(0.04);
    //
    //    F0 = mix(F0, albedo, metallic);
    //
    //    vec3 Lo = vec3(0.0);
    //
    //    for(int i = 0; i < 4; ++i)
    //    {
    //        vec3 L = normalize(vec3(0.0, 0.0, -1.0));
    //        vec3 H = normalize(V + L);
    //        float distance = length(vec3(0.0, 0.0, 1.0) - vs_out.position);
    //        float attenuation = 1.0 / (distance * distance);
    //        vec3 radiance = vec3(1.0);
    //
    //        float NDF = distribution_ggx(N, H, roughness);
    //        float G = geometry_smith(N, V, L, roughness);
    //        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);
    //
    //        vec3 nominator = NDF * G * F;
    //        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    //        vec3 specular = nominator / max(denominator, 0.001);
    //
    //        vec3 kS = F;
    //        vec3 kD = vec3(1.0) - kS;
    //        kD *= 1.0 - metallic;
    //
    //        float NdotL = max(dot(N, L), 0.0);
    //
    //        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    //    }

    FragColor = vec4(finalColor, 1.0);
}