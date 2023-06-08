#version 460

out vec4 FragColor;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
    vec3 bitangent;
} vs_out;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}