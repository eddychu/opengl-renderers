#version 460
out vec4 FragColor;

in vec2 TexCoords;

layout(binding = 0)uniform sampler2D gPosition;
layout(binding = 1)uniform sampler2D gNormal;
layout(binding = 2)uniform sampler2D gAlbedoSpec;

struct PointLight {
    vec4 position;
    vec4 color;
    vec4 radius;
};

layout(std140, binding=0) uniform PointLights {
    PointLight pointLights[200];
};


void main() {
    vec3 position = texture(gPosition, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;

    vec3 diffuse = vec3(0.0);
    for(int i = 0; i < 200; i++) {
        vec3 lightPos = pointLights[i].position.xyz;
        vec3 lightColor = pointLights[i].color.xyz;
        float lightRadius = pointLights[i].radius.w;
        float lightIntensity = pointLights[i].radius.x;

        vec3 lightDir = normalize(lightPos - position);
        float lightDistance = length(lightPos - position);
        

        float attenuation = 1.0 /(5.0 + (lightDistance * lightDistance));

        float NdotL = max(dot(normal, lightDir), 0.0);
        diffuse += lightColor * lightIntensity * NdotL * attenuation;
    }

    FragColor = vec4(diffuse * albedo, 1.0);
}