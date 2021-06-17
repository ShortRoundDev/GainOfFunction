#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;
out vec3 dist;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 playerPos;
uniform vec2 scale;
uniform vec3 offset;

void main() {
    vec3 translatedPos = vec3(aPos.xy * scale, aPos.z) + offset;
    gl_Position = projection * view * vec4(translatedPos, 1.0f);
    TexCoord = vec2(aTex.x, aTex.y);
    dist = translatedPos - playerPos;
}