#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform vec3 trans;

void main() {
    vec3 translatedPos = (aPos * trans.z);
    translatedPos = vec3(translatedPos.x + trans.x, translatedPos.y + trans.y, aPos.z);
    gl_Position = vec4(translatedPos, 1.0f);
    TexCoord = vec2(aTex.x, 1.0f - aTex.y);
}