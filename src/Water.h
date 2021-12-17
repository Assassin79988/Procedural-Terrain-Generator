#ifndef WATER_H_
#define WATER_H_

#include "PerlinNoise.h"
#include "Icosphere.h"
#include "loadTexture.h"

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

// This class define water on a planet
class Water {
private:
	Icosphere* mesh;
	float radius;
	
	std::unique_ptr<Shader> shader;
	std::unique_ptr<GPUMesh> glMesh;
	std::unique_ptr<RGBA8Texture> texture;

	float timer;
public:
	Water(float radius, Vec3 center, int lod);

	void init();
	void draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp, std::vector<float> planetHeightMap, std::vector<Vec3> pVertices, std::vector<Vec3> pVNormals);

	std::string load_source(const char* fname) {
		std::ifstream f(fname);
		std::stringstream buffer;
		buffer << f.rdbuf();
		return buffer.str();
	}
};

Water::Water(float radius, Vec3 center, int lod) {
	this->radius = radius;
	mesh = new Icosphere(center, radius, lod);
}

void Water::init() {
	shader = std::unique_ptr<Shader>(new Shader());
	glMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
	texture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());

	shader->verbose = true;
	shader->add_vshader_from_source(load_source("Shaders/water_vshader.glsl").c_str());
	shader->add_fshader_from_source(load_source("Shaders/water_fshader.glsl").c_str());
	shader->link();

	loadTexture(texture, "water.png");
	texture->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Water::draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp, std::vector<float> planetHeightMap, std::vector<Vec3> pVertices, std::vector<Vec3> pVNormals) {
	std::vector<unsigned int> triangle_indices = mesh->genMesh();
	std::vector<Vec3> vertices = mesh->getVertices();
	std::vector<Vec3> vnormals = mesh->getVertexNormals();
	float radius = mesh->getRadius();

	glMesh->set_vbo<Vec3>("vposition", vertices);
	glMesh->set_vbo<Vec3>("vnormal", vnormals);

	glMesh->set_vbo<Vec3>("pvnormal", pVNormals);
	glMesh->set_vbo<Vec3>("pvposition", pVertices);
	glMesh->set_vbo<float>("pvheight", planetHeightMap);

	glMesh->set_triangles(triangle_indices);

	shader->bind();

	shader->set_uniform("radius", radius);

	timer += 0.1;
	shader->set_uniform("time", timer);

	Vec3 cameraCenter = cameraPos;

	shader->set_uniform("viewer", cameraCenter);
	shader->set_uniform("viewerDir", cameraFront.normalized());

	shader->set_uniform("center", mesh->getCenter());

	Mat4x4 M = Mat4x4::Identity();
	shader->set_uniform("M", M);

	Vec3 center = cameraPos + cameraFront;

	Mat4x4 V = lookAt(cameraPos, center, cameraUp);
	shader->set_uniform("V", V);

	Mat4x4 P = perspective(fov, SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 100.0f);
	shader->set_uniform("P", P);

	glActiveTexture(GL_TEXTURE0);
	texture->bind();
	shader->set_uniform("waterTex", 0);

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

	glDisable(GL_BLEND);

	shader->unbind();
}

#endif