#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 dist;

uniform sampler2D tex;
uniform vec4 tint;
uniform float minBright;

void main(){
    
    vec4 t = texture(tex, TexCoord);
    if(t.a == 0.0)
        discard;
    float z = min(1,
        max(
            minBright,
            round(10.0/(length(dist.xz)) - 1)/10.0
        )
    );
    FragColor = vec4(t.b, t.g, t.r, t.a) * vec4(z, z, min(1.0, z * 1.2), 1.0) * tint;
}