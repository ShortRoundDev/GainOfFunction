#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex;
uniform float frame;
uniform float maxFrame;

void main(){
    
    vec4 t = texture(tex, vec2((TexCoord.x/maxFrame) + (frame/maxFrame), TexCoord.y));
    if(t.a == 0.0)
        discard;
    FragColor = vec4(t.b, t.g, t.r, t.a);
}