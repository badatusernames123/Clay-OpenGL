#version 330 core
in vec2 frag_uv;
in vec4 frag_color;
out vec4 color;

uniform sampler2D tex_sampler;

void main()
{    
    vec4 tex_color = texture(tex_sampler, frag_uv);
    color = frag_color * tex_color;
}  