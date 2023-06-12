#version 460
in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
} vs_out;

struct PointLight {
    vec4 position;
    vec4 color;
    vec4 radius;
};

struct VisibleIndex {
    int index;
};

layout(std430, binding = 0) readonly buffer LightBuffer {
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer {
	VisibleIndex data[];
} visibleLightIndicesBuffer;

layout(binding = 0) uniform sampler2D albedoMap;


uniform int numberOfTilesX;

uniform vec3 cameraPosition;

out vec4 fragColor;

float attenuation(vec3 lightDirection, float radius) {
    float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

void main() {
    vec3 albedo = texture(albedoMap, vs_out.uv).rgb;

    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * numberOfTilesX + tileID.x;

    vec3 viewDirection = normalize(cameraPosition - vs_out.position);
    vec3 color = vec3(0.0);
    uint offset = index * 1024;
    for (uint i = 0;
     i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++) {
        uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
        PointLight light = lightBuffer.data[lightIndex];
        vec4 lightColor = light.color;
        vec3 lightL = light.position.xyz - vs_out.position;
        float lightRadius = light.radius.w;
        float lightAttenuation = attenuation(lightL, lightRadius);

        vec3 lightDirection = normalize(lightL);

        vec3 halfVector = normalize(lightDirection + viewDirection);

        float diffuse = max(dot(vs_out.normal, lightDirection), 0.0);

        color.rgb += diffuse * lightColor.rgb * lightAttenuation;
     }

     vec3 final = albedo * color;

    fragColor = vec4(final, 1.0);
}