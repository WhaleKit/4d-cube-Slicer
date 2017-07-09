#version 330 core

out vec4 color;
in float depth;
in vec3 normal;
void main()
{
    vec3 myNormal;
    myNormal = float (gl_FrontFacing) * (-normal)
             + float(!gl_FrontFacing) * ( normal);
    vec3 lihtSource = vec3(0.2f, 0.9f, -0.4f)*2;
//    color = mix(vec4(0.02f, 0.4f, 0.9f, 0.5f)
//                    *max(dot(normal,lihtSource),0.3),
//                vec4(0.1f, 0.1f, 0.1f, 0.1f), depth/5);
    color = vec4(0.02f, 0.4f, 0.9f, 0.5f)
                *(dot(myNormal,lihtSource));
    color += vec4(0.02f, 0.4f, 0.9f, 0.5f)*0.2;
}
