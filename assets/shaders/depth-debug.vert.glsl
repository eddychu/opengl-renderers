#version 460
layout(location = 0) in vec3 POSITION;
layout(location = 2) in vec2 TEXCOORD;

out vec2 v_texCoord;

void main() {
    gl_Position = vec4(POSITION, 1.0);
    v_texCoord = TEXCOORD;
}