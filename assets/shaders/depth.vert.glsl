#version 460

layout(location = 0) in vec3 POSITION;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(POSITION, 1.0);
}