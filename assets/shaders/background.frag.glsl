#version 460 core
out vec4 FragColor;

layout(binding = 0) uniform samplerCube SKYBOX_MAP;

in vec3 TEX_COORDS;

void main()
{    
    FragColor = texture(SKYBOX_MAP, TEX_COORDS);
}