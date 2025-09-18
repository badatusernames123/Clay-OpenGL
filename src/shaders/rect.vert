#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;
out vec4 frag_color;
out vec2 frag_uv;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    frag_uv = uv;
    frag_color = color;
}  