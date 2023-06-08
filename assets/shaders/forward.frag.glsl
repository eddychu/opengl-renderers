#version 460

out vec4 FragColor;

layout(binding = 0) uniform sampler2D baseColorTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D metallicRoughnessTexture;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec4 position;
    vec4 color;
    vec4 radius;
};

layout(std140, binding=0) uniform PointLights {
    PointLight pointLights[200];
};


in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
} vs_out;

void main()
{
    vec3 diffuse = vec3(0.0);
    for(int i = 0; i < 200; i++) {
        vec3 lightPos = pointLights[i].position.xyz;
        vec3 lightColor = pointLights[i].color.xyz;
        float lightRadius = pointLights[i].radius.w;
        float lightIntensity = pointLights[i].radius.x;

        vec3 lightDir = normalize(lightPos - vs_out.position);
        float lightDistance = length(lightPos - vs_out.position);
        

        float attenuation = 1.0 /(1.0 + (lightDistance * lightDistance));

        float NdotL = max(dot(normalize(vs_out.normal), lightDir), 0.0);
        diffuse += lightColor * lightIntensity * NdotL * attenuation;
    }

    vec3 albedo = texture(baseColorTexture, vs_out.uv).rgb;

    FragColor = vec4(albedo * diffuse, 1.0);
}