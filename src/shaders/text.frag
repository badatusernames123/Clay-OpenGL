#version 330 core
in vec2 frag_uv;
out vec4 color;

uniform sampler2D character_atlas;
uniform vec4 text_color;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(character_atlas, frag_uv).r);
    color = text_color * sampled;
}  