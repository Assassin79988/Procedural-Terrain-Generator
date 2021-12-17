#version 330 core

in vec3 vposition;
in vec3 vnormal;
in vec3 vsurfacenormal;
in vec2 vtexcoord;
in float vheight;

uniform float radius;
uniform vec3 center;
uniform vec3 viewer;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec2 uv;
out vec3 fragPos;
out vec3 vs;
out float fheight;
out vec3 fnormal;

void main() {
    vs = vsurfacenormal;
    fnormal = vnormal;
    fheight = vheight;
   
    fragPos =  vposition + vnormal * vheight;

    // Set gl_Position
    gl_Position = P*V*M*vec4(fragPos, 1.0f);

}

	