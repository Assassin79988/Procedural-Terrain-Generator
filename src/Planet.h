#ifndef PLANET_H_
#define PLANET_H_

#include "PerlinNoise.h"
#include "Icosphere.h"
#include "Water.h"
#include "loadTexture.h"

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

// This class defines a planet
class Planet {
private:
	Icosphere* mesh = nullptr;
	Water* water = nullptr;

	std::unique_ptr <RGBA8Texture> sandTexture;
	std::unique_ptr <RGBA8Texture> grassTexture;
	std::unique_ptr <RGBA8Texture> rockTexture;
	std::unique_ptr <RGBA8Texture> snowTexture;

	std::vector<Vec3> planetSurfaceNormals;
	std::vector<float> heightMap;

	std::unique_ptr<Shader> shader;
	std::unique_ptr<GPUMesh> glMesh;

	float smax(float a, float b, float t);
	float lerp(float a, float b, float t);

public:
	Planet(Icosphere* mesh);

	void setMesh(Icosphere* mesh) { 
		this->mesh = mesh; 
		calcHeightMap();
		calcSurfaceNormals();
	}

	Icosphere* getMesh() { return this->mesh;  }

	void calcHeightMap();
	std::vector<float> getHeightMap() { return this->heightMap; };

	void calcSurfaceNormals();
	std::vector<Vec3> getSurfaceNormals() { return this->planetSurfaceNormals; }

	void init();
	void draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp);

	std::string load_source(const char* fname) {
		std::ifstream f(fname);
		std::stringstream buffer;
		buffer << f.rdbuf();
		return buffer.str();
	}
};

Planet::Planet(Icosphere* mesh) {
	this->mesh = mesh;
	water = new Water(mesh->getRadius() * 1.02, mesh->getCenter(), 5);

	sandTexture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
	grassTexture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
	rockTexture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
	snowTexture =std::unique_ptr<RGBA8Texture>(new RGBA8Texture());

	calcHeightMap();
	calcSurfaceNormals();
}

float Planet::smax(float a, float b, float t) {
	return log(exp(a * t) + exp(a * t) - 1.0f) / t;
}

float Planet::lerp(float a, float b, float t) {
	return a * t + (1 - t) * b;
}

void Planet::calcHeightMap() {
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> seed(0, 100000);
	PerlinNoise noise = PerlinNoise(2048, 2048, 8, 2, 0.9, 0.0, 512, seed(rd));
	float period = 20.0f;

	std::vector<Vec3> vertices = mesh->getVertices();


	for (int i = 0; i < vertices.size(); ++i) {
		Vec3 coord = vertices[i];
		float perlin_noise = noise.fBm(coord / period);
		float continent = noise.hybridMultifractal(coord / period) * 0.2;

		if (perlin_noise > -0.1f) {
			perlin_noise += lerp(0, continent, perlin_noise);
		}

		if (perlin_noise > 0.4) {
			perlin_noise += lerp(0.0, 0.3, (perlin_noise - 0.4));
		}

		heightMap.push_back(perlin_noise * powf(mesh->getRadius(), 0.5));
	}
}

void Planet::calcSurfaceNormals() {
	std::vector<Face> faces = mesh->getFaces();
	std::vector<Vec3> vertices = mesh->getVertices();
	std::vector<Vec3> vnormals = mesh->getVertexNormals();

	for (int i = 0; i < vertices.size(); ++i) {
		Vec3 avg = Vec3(0, 0, 0);
		int num = 0;
		for (int j = 0; j < faces.size(); ++j) {
			int a = faces[j].vertices[0];
			int b = faces[j].vertices[1];
			int c = faces[j].vertices[2];

			if (a == i) {
				Vec3 tempA = vertices[a] + vnormals[a] * heightMap[a];
				Vec3 tempB = (vertices[b] + vnormals[b] * heightMap[b]);
				Vec3 tempC = (vertices[c] + vnormals[c] * heightMap[c]);
				avg += (tempB - tempA).cross(tempC - tempA);
				num++;
			}

			if (b == i) {
				Vec3 tempA = vertices[a] + vnormals[a] * heightMap[a];
				Vec3 tempB = (vertices[b] + vnormals[b] * heightMap[b]);
				Vec3 tempC = (vertices[c] + vnormals[c] * heightMap[c]);
				avg += (tempB - tempA).cross(tempC - tempA);
				num++;
			}

			if (c == i) {
				Vec3 tempA = vertices[a] + vnormals[a] * heightMap[a];
				Vec3 tempB = (vertices[b] + vnormals[b] * heightMap[b]);
				Vec3 tempC = (vertices[c] + vnormals[c] * heightMap[c]);
				avg += (tempB - tempA).cross(tempC - tempA);
				num++;
			}
		}

		avg /= (float)num;
		avg = avg.normalized();

		planetSurfaceNormals.push_back(avg);
	}
}

void Planet::init() {
	shader = std::unique_ptr<Shader>(new Shader());
	glMesh = std::unique_ptr<GPUMesh>(new GPUMesh());

	shader->verbose = true;
	shader->add_vshader_from_source(load_source("Shaders/terrain_vshader.glsl").c_str());
	shader->add_fshader_from_source(load_source("Shaders/terrain_fshader.glsl").c_str());
	shader->link();

	water->init();

	loadTexture(sandTexture, "sand.png");
	sandTexture->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	loadTexture(grassTexture, "grass.png");
	grassTexture->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	loadTexture(rockTexture, "rock.png");
	rockTexture->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	loadTexture(snowTexture, "snow.png");
	snowTexture->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Planet::draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp) {
	std::vector<unsigned int> triangle_indices = mesh->genMesh();
	std::vector<Vec3> vertices = mesh->getVertices();
	std::vector<Vec3> vnormals = mesh->getVertexNormals();
	std::vector<Vec2> uvs = mesh->getUvs();

	float radius = mesh->getRadius();

	glMesh->set_vbo<Vec3>("vposition", vertices);
	glMesh->set_vbo<Vec3>("vnormal", vnormals);
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

	Mat4x4 P = perspective(fov, SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 100.0f);
	shader->set_uniform("P", P);

	glMesh->set_vbo<float>("vheight", heightMap);
	glMesh->set_vbo<Vec3>("vsurfacenormal", planetSurfaceNormals);

	glActiveTexture(GL_TEXTURE0);
	sandTexture->bind();
	shader->set_uniform("snad", 0);

	glActiveTexture(GL_TEXTURE1);
	grassTexture->bind();
	shader->set_uniform("grass", 1);

	glActiveTexture(GL_TEXTURE2);
	rockTexture->bind();
	shader->set_uniform("rock", 2);

	glActiveTexture(GL_TEXTURE3);
	snowTexture->bind();
	shader->set_uniform("snow", 3);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMesh->set_attributes(*shader);
	glMesh->set_mode(GL_TRIANGLES);

	// Wirefrane 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

	glMesh->set_attributes(*shader);
	glMesh->draw();

	shader->unbind();

	water->draw(fov, cameraPos, cameraFront, cameraUp, heightMap, vertices, vnormals);
}

#endif