#version 330 core

out vec4 color;
in float depth;
void main()
{
    color = mix(vec4(0.02f, 0.4f, 0.9f, 0.5f), vec4(0.1f, 0.1f, 0.1f, 0.1f), depth/5);
}
