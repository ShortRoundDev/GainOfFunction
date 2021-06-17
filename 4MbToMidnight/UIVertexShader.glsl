#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform vec3 scale;
uniform vec3 offset;

void main() {
    gl_Position = vec4((aPos * scale) + offset, 1.0);
    TexCoord = vec2(aTex.x, aTex.y);
}