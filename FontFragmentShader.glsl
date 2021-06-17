#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform int c;
uniform sampler2D tex;

void main(){
    vec4 t = texture(tex, vec2(TexCoord.x + (c/36.0), TexCoord.y));
    if(t.a == 0.0)
        discard;
    FragColor = t;
}