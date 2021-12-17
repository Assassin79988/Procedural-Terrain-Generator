#version 330 core
in vec3 vposition;
in float noise;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 fcolor;

void main() {   
    vec4 col = mix(vec4(1.00, 0.78, 0.00, 1.0), vec4(1.00, 0.44, 0.00, 1.0), noise);

    fcolor = col;

    // Set gl_Position
    gl_Position = P*V*M*vec4(vposition, 1.0f);

}
