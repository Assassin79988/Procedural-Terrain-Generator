#version 330 core
in vec3 vposition;

out vec3 TexCoords;

uniform mat4 V;
uniform mat4 P;

void main()
{
    TexCoords = vec3(vposition.x, -vposition.z, -vposition.y);
    gl_Position = P*V*vec4(vposition, 1.0);
}  