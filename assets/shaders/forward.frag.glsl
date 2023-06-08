#version 460

out vec4 FragColor;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
} vs_out;

void main()
{
    FragColor = vec4(vs_out.normal * 0.5 + 0.5, 1.0);
}