#version 330 core

#define M_PI 3.1415926535897932384626433832795

out vec4 color;

uniform vec3 viewer;

uniform sampler2D sand;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D snow;

in vec3 fragPos;
in vec3 vs;
in float fheight;
in vec3 fnormal;

uniform float radius;
uniform vec3 center;
const float  sandLevel = 0.5;
const float  grassLevel = 0.7;
const float  rockLevel = 0.9;
const float  snowLevel = 0.95;

void main() {
    vec4 col = vec4(0,0,0,1.0);
    //light position
    vec3 lightPos = vec3(50.0f, -50.0f, 200.0f);
    vec3 viewPos = viewer;

    vec3 normal = normalize(vs);

	float u = (atan(fnormal.x, fnormal.z) / M_PI) / 2.f + 0.5f;
    float v = (asin(-fnormal.y) / (M_PI / 2.f)) / 2.f + 0.5f;

    vec2 uv = vec2(u, v);

    float height = fheight / (radius / 10.0f);

    vec3 refNormal = normalize((fragPos - center));
    float slope = dot(refNormal, normal);
    
    if (fheight < 0.0f) {
        col = texture(sand, vec2(u, v));
    } else if (fheight < 0.4f * pow(radius, 0.5)) {
        col = texture(grass, vec2(u, v));
        if (slope > 0.6f && fheight < 0.2f * pow(radius, 0.5)) {
            float scale = (slope - 0.6) / 0.6;
             col = (texture(grass, vec2(u,v)) * (scale))+(texture(sand, vec2(u,v)) * (1 - scale));
        }
    } else if (fheight < 0.8f * pow(radius, 0.5)){
        col = texture(rock, vec2(u, v));
        if (slope > 0.6f && fheight < 0.6f * pow(radius, 0.5)) {
            float scale = (slope - 0.6) / 0.6;
            col = (texture(rock, vec2(u,v)) * (scale))+(texture(grass, vec2(u,v)) * (1 - scale));
        }
    } else {
        col = texture(snow, vec2(u, v));
        if (slope > 0.6f && fheight < 0.9f * pow(radius, 0.5)) {
            float scale = (slope - 0.6) / 0.6;
            col = (texture(snow, vec2(u,v)) * (scale))+(texture(rock, vec2(u,v)) * (1 - scale));
        }
    }

	// Calculate ambient lighting factor
    float ambient = 0.05f;
    float diffuse_coefficient = 0.2f;
    float specular_coefficient = 0.2f;
    float specularPower = 16.0;

    // Calculate diffuse lighting factor
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = diffuse_coefficient * max(0.0f, -dot(normal, lightDir));

    // Calculate specular lighting factor
    vec3 view_direction = normalize(viewPos - fragPos);
    vec3 halfway = normalize(lightDir + view_direction);
    float specular = specular_coefficient * max(0.0f, pow(dot(normal, halfway), specularPower));
	
    //vec4 col = vec4(0,0.5,0.1,1);    
    col += (ambient + diffuse + specular);  

    color = vec4(col);
    
}
