#version 330 core

layout(triangles) in;

layout(triangle_strip, max_vertices = 3) out;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;

void main()
{
    vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    normal = -normalize(cross(A,B));

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
