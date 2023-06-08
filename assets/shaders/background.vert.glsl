#version 460

layout(location=0) in vec3 POSITION;

uniform mat4 VP;

out vec3 TEX_COORDS;

void main() {
    vec4 pos = VP * vec4(POSITION, 1.0);
    gl_Position = pos.xyww;
    TEX_COORDS = POSITION;
}