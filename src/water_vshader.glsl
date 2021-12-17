#version 330 core

uniform sampler2D waterTex;

in vec3 vposition;
in vec3 vnormal;

in vec3 pvnormal;
in vec3 pvposition;
in float pvheight;

uniform float radius;
uniform vec3 viewer;
uniform vec3 viewerDir;
uniform vec3 center;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

const vec4 colA = vec4(0.0, 0.0, 0.5, 0.3);
const vec4 colB = vec4(0.0, 0.0, 1.0, 0.6);

const float multiplierDepth= 1.1f;
const float multiplierAlpha = 0.1f;

out vec4 vcolor;
out vec3 fnormal;

bool intsertRaySphere(vec3 o, vec3 d, vec3 c, float r, out float t, out vec3 intersect) {
    vec3 m = o - center;

    float A = length(d) * length(d);
	float B = 2 * dot(d, m);
	float C = length(m) * length(m) - r * r;

	float discriminant = B * B - 4 * A * C;

	bool hasIntersect = discriminant >= 0.00001f;

	float t1;
	float t2;
	if (!hasIntersect) 
		return hasIntersect;

	t1 = (-B + sqrt(discriminant)) / (2.0f * A);
	t2 = (-B - sqrt(discriminant)) / (2.0f * A);
		
	t = t1;

	if (t1 < t2)
		t = t2;

	intersect = o + t * d;

	return hasIntersect;

}

void main() {
	fnormal = vnormal;
    float pvradius = length((pvposition + pvnormal * pvheight) - center);

	float t;
	float tp;
	vec3 intersectionPoint;
	vec3 planetIntersectionPoint;

	bool hasHit = intsertRaySphere(viewer, normalize(vposition - viewer), center, radius, t, intersectionPoint);
	bool hasHitPlanet = intsertRaySphere(viewer, normalize(vposition - viewer), center, pvradius, tp, planetIntersectionPoint);

	vcolor = colB;
	float depth;
	if (hasHit && hasHitPlanet) {
		depth = t - tp;
		if (depth > 0.00001f) {
			float opticalDepth = 1 - exp(-depth * multiplierDepth);
			float alpha = 1 - exp(-depth *multiplierAlpha);
			vec4 oceanColor = mix(colA, colB, opticalDepth);
			vcolor = mix(colB, oceanColor, alpha);
		}
	}

    gl_Position = P*V*M*vec4(vposition, 1.0f);

}
