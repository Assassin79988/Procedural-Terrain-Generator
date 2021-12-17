#ifndef SUN_H_
#define SUN_H_

#include <fstream>

#include "PerlinNoise.h"
#include "Icosphere.h"
#include "loadTexture.h"

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

// This class defines a sun object
class Sun {
private:
	// stores the mesh and other infomation
	Icosphere* mesh;
	std::vector<float> noiseMap;
	std::unique_ptr<Shader> shader;
	std::unique_ptr<GPUMesh> glMesh;

	// Stores the max/min noise value
	float min;
	float max;

	// timer variable
	float timer = 0;

	// Loads a file
	std::string load_source(const char* fname) {
		std::ifstream f(fname);
		std::stringstream buffer;
		buffer << f.rdbuf();
		return buffer.str();
	}
public:
	Sun(Icosphere* mesh);

	void updateTexture();
	void init();
	void draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp);

};

Sun::Sun(Icosphere* mesh) {
	this->mesh = mesh;
}

// Updates the suns texture
void Sun::updateTexture() {
	noiseMap.clear();
	PerlinNoise noise = PerlinNoise(2048, 2048, 8, 2, 0.9, 0.0, 512, 2021);
	float period = 20.0f;

	std::vector<Vec3> vertices = mesh->getVertices();

	min = 10000;
	max = -10000;

	timer += 0.1;
	for (int i = 0; i < vertices.size(); ++i) {
		Vec3 pos = Vec3(vertices[i][0] + timer, vertices[i][1] + timer, vertices[i][2] + timer);
		float val = noise.hybridMultifractal(pos / period);

		if (val > max) max = val;
		if (val < min) min = val;

		noiseMap.push_back(val);
	}

	for (int i = 0; i < noiseMap.size(); ++i) {
		noiseMap[i] = (noiseMap[i] - min) / (max - min);
	}
}

// Loads and init infomation for the render
void Sun::init() {
	shader = std::unique_ptr<Shader>(new Shader());
	glMesh = std::unique_ptr<GPUMesh>(new GPUMesh());

	shader->verbose = true;
	shader->add_vshader_from_source(load_source("Shaders/sun_vshader.glsl").c_str());
	shader->add_fshader_from_source(load_source("Shaders/sun_fshader.glsl").c_str());
	shader->link();
}

// Draws the sun
void Sun::draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp) {
	updateTexture();

	std::vector<unsigned int> triangle_indices = mesh->genMesh();
	std::vector<Vec3> vertices = mesh->getVertices();
	std::vector<Vec3> vnormals = mesh->getVertexNormals();
	std::vector<Vec2> uvs = mesh->getUvs();

	float radius = mesh->getRadius();

	glMesh->set_vbo<Vec3>("vposition", vertices);
	glMesh->set_vbo<Vec3>("vnormal", vnormals);
	glMesh->set_vbo<float>("noise", noiseMap);
	glMesh->set_vtexcoord(uvs);
	glMesh->set_triangles(triangle_indices);

	shader->bind();

	shader->set_uniform("radius", radius);
	shader->set_uniform("center", mesh->getCenter());
	shader->set_uniform("viewer", cameraPos);

	Mat4x4 M = Mat4x4::Identity();
	shader->set_uniform("M", M);

	Vec3 center = cameraPos + cameraFront;

	Mat4x4 V = lookAt(cameraPos, center, cameraUp);
	shader->set_uniform("V", V);

	Mat4x4 P = perspective(fov, SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 300.0f);
	shader->set_uniform("P", P);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMesh->set_attributes(*shader);
	glMesh->set_mode(GL_TRIANGLES);

	// Wirefrane 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

	glMesh->set_attributes(*shader);
	glMesh->draw();

	shader->unbind();
}

#endif