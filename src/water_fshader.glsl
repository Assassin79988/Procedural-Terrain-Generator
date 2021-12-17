#version 330 core

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D waterTex;

in vec4 vcolor;
in vec3 fnormal;
in vec3 vposition;

uniform vec3 viewer;
uniform float radius;
uniform float time;

out vec4 color;

void main() {
    vec4 col = vcolor;  

    // computes yv coordinates
    float u = (atan(fnormal.x, fnormal.z) / M_PI) / 2.f + 0.5f;
    float v = (asin(-fnormal.y) / (M_PI / 2.f)) / 2.f + 0.5f;

    // displaces water texture
    vec4 texCol = vec4(0, 0, 0, 0.0);
    texCol  +=  texture2D(waterTex, (vec2(u, v) - 0.5) * (sin(time) + 1.0) / 2.0 + 0.5);
    texCol  +=  texture2D(waterTex, (vec2(u, v) - 0.5) * (cos(2 * time) + 1.0) / 2.0 + 0.5);
    texCol  +=  texture2D(waterTex, (vec2(u, v) - 0.5) * (sin(3 * time) + 1.0) / 2.0 + 0.5);
    texCol  +=  texture2D(waterTex, (vec2(u, v) - 0.5) * (cos(4 * time) + 1.0) / 2.0 + 0.5);
    texCol /= 4;
    col += texCol;

    //light position
    vec3 lightPos = vec3(50.0f, -50.0f, 200.0f);
    vec3 viewPos = viewer;

    vec3 normal = fnormal;
	
	// Calculate ambient lighting factor
    float ambient = 0.05f;
    float diffuse_coefficient = 0.2f;
    float specular_coefficient = 0.4f;
    float specularPower = 16.0;

    // Calculate diffuse lighting factor
    vec3 lightDir = normalize(lightPos - vposition);
    float diffuse = diffuse_coefficient * max(0.0f, -dot(normal, lightDir));

    // Calculate specular lighting factor
    vec3 view_direction = normalize(viewPos - vposition);
    vec3 halfway = normalize(lightDir + view_direction);
    float specular = specular_coefficient * max(0.0f, pow(dot(normal, halfway), specularPower));
	   
    col += (ambient + diffuse + specular);  

    col.w = vcolor.w;

    color = vec4(col);
}