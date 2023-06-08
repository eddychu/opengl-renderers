#version 460 core


layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;


layout(binding = 0) uniform sampler2D baseColorTexture;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
} vs_out;

void main()
{
    gPosition = vs_out.position;
    gNormal = normalize(vs_out.normal);
    gAlbedo = texture(baseColorTexture, vs_out.uv).rgb;
}