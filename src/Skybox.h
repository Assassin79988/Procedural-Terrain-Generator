#ifndef SKYBOX_H_
#define SKYBOX_H_

#include<cstdlib>
#include <iostream>
#include <vector>
#include <random>

#include "Image.h"
#include "PerlinNoise.h"
#include "Icosphere.h"
#include "loadTexture.h"

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

// This class defines skybox that is procedurally generated using perlin noise
// as a cube map
class Skybox {
private:
	// Id for cube mesh texture
	GLuint skyboxID;

	// size of each tile
	int size;
	// Defines the properties of the space background
	float starDensity, brightness;
	float nebulaeDensity, falloff;
	float period;

	// Stores the color for a nebulae
	Vec3 nebulaeColor = Vec3(75, 0, 130);

	// Shader and GPUMesh
	std::unique_ptr<Shader> shader;
	std::unique_ptr<GPUMesh> glMesh;

	// Stores the image file names
	std::vector<std::string> textureFiles = {"./skybox_front.png", "./skybox_back.png", "./skybox_down.png", "./skybox_up.png", "./skybox_right.png", "./skybox_left.png"};
	std::vector<Vec3> image_front;
	std::vector<Vec3> image_back;
	std::vector<Vec3> image_left;
	std::vector<Vec3> image_right;
	std::vector<Vec3> image_up;
	std::vector<Vec3> image_down;

	void generateSkybox();
	void generateStars(std::vector<Vec3>& image);
	void generateNebulae();
	Vec3 lerp(Vec3 a, Vec3 b, float t) {
		return a * t + (1 - t) * b;
	}
	std::string load_source(const char* fname) {
		std::ifstream f(fname);
		std::stringstream buffer;
		buffer << f.rdbuf();
		return buffer.str();
	}
public:
	Skybox(int size);

	void saveimg();
	unsigned int loadCubemap();
	void draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp);
	void init();
	void genMesh();
	void getNebulaeColor();

};

Skybox::Skybox(int size) {
	// Create random number generators for each property of the skymap
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_real_distribution<float> randStarDensity(0.01f, 0.1f);
	std::uniform_real_distribution<float> randBrightness(0.01f, 0.5f);
	std::uniform_real_distribution<float> randNebulaeDensity(0.01, 0.4);
	std::uniform_real_distribution<float> randFalloff(2.0, 8.0);
	std::uniform_real_distribution<float> randPeriod(100, 300);

	// Sets the value for each property
	this->size =size;
	this->starDensity = randStarDensity(e1);
	this->brightness = randBrightness(e1);
	this->nebulaeDensity = randNebulaeDensity(e1);
	this->falloff = randFalloff(e1);
	this->period = randPeriod(e1);

	// sets the color to black for each texture
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			image_front.push_back(Vec3(0.0, 0.0, 0.0));
			image_back.push_back(Vec3(0.0, 0.0, 0.0));
			image_left.push_back(Vec3(0.0, 0.0, 0.0));
			image_right.push_back(Vec3(0.0, 0.0, 0.0));
			image_up.push_back(Vec3(0.0, 0.0, 0.0));
			image_down.push_back(Vec3(0.0, 0.0, 0.0));
		}
	}

	generateSkybox();
	saveimg();
}

// Loads the shaders + inits objects i.e. Shader and GPUMesh
void Skybox::init() {
	loadCubemap();

	shader = std::unique_ptr<Shader>(new Shader());
	glMesh = std::unique_ptr<GPUMesh>(new GPUMesh());

	shader->verbose = true;
	shader->add_vshader_from_source(load_source("Shaders/skybox_vshader.glsl").c_str());
	shader->add_fshader_from_source(load_source("Shaders/skybox_fshader.glsl").c_str());
	shader->link();

	genMesh();
}

// Creates the mesh for the skybox
void Skybox::genMesh() {
	std::vector<Vec3> points;
	float size = 100.0f;
	points.push_back(Vec3(1, 1, 1) * size); // 0
	points.push_back(Vec3(-1, 1, 1) * size); // 1
	points.push_back(Vec3(1, 1, -1) * size); // 2
	points.push_back(Vec3(-1, 1, -1) * size); // 3
	points.push_back(Vec3(1, -1, 1) * size); // 4
	points.push_back(Vec3(-1, -1, 1) * size); // 5
	points.push_back(Vec3(-1, -1, -1) * size); // 6
	points.push_back(Vec3(1, -1, -1) * size); // 7

	std::vector<unsigned int> indices = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
	glMesh->set_vbo<Vec3>("vposition", points);
	glMesh->set_triangles(indices);
}

// Loads the cube map
unsigned int Skybox::loadCubemap() {
	glGenTextures(1, &skyboxID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);

	for (unsigned int i = 0; i < textureFiles.size(); i++) {
		std::vector<unsigned char> image;
		loadTexture(image, textureFiles[i].c_str());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return skyboxID;
}

void Skybox::draw(float fov, Vec3 cameraPos, Vec3 cameraFront, Vec3 cameraUp) {
	shader->bind();

	Vec3 center = cameraPos + cameraFront;

	Mat4x4 V = lookAt(cameraPos, center, cameraUp);
	// Removes translation part of the view matrix
	V(0, 3) = 0;
	V(1, 3) = 0;
	V(2, 3) = 0;
	V(3, 3) = 1;
	shader->set_uniform("V", V);

	Mat4x4 P = perspective(fov, SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 500 * 2.0f);
	shader->set_uniform("P", P);

	glActiveTexture(GL_TEXTURE_CUBE_MAP);
	shader->set_uniform("skybox", 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glMesh->set_attributes(*shader);
	glMesh->set_mode(GL_TRIANGLE_STRIP);
	glDepthFunc(GL_LESS);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(999999);
	glMesh->draw();
	shader->unbind();
}

void Skybox::generateSkybox() {
	// Generate stars for each face of the cube map
	generateStars(image_front);
	generateStars(image_back);
	generateStars(image_left);
	generateStars(image_right);
	generateStars(image_up);
	generateStars(image_down);

	// generates the nebulae
	getNebulaeColor();
	generateNebulae();
}

// Picks a random color for the nebula
void Skybox::getNebulaeColor() {
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(20, 200);

	nebulaeColor = Vec3(uniform_dist(e1), uniform_dist(e1), uniform_dist(e1));
}

// Generates the background stars
void Skybox::generateStars(std::vector<Vec3>& image) {
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(0, size * size - 1);

	int numStars = (int)(size * size * starDensity);

	for (int i = 0; i < numStars; ++i) {
		int pt = uniform_dist(e1);
		int color = (int)(255 * log(1 - rand() / (float)RAND_MAX) * -brightness);
		image[pt][0] = color;
		image[pt][1] = color;
		image[pt][2] = color;
	}

}

// generates the nebule
void Skybox::generateNebulae() {
	PerlinNoise noise = PerlinNoise(size, size, 8, 2, 0.9, 0.0, 128, 20202);

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			float noise_val = (noise.fBm(Vec3(x / period, y / period, 0)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_down[x + y * size] = lerp(nebulaeColor, image_down[x + y * size], val);

		}
	}

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			float noise_val = (noise.fBm(Vec3(x / period, y / period, size / period)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_up[x + y * size] = lerp(nebulaeColor, image_up[x + y * size], val);

		}
	}

	for (int z = 0; z < size; ++z) {
		for (int x = 0; x < size; ++x) {
			float noise_val = (noise.fBm(Vec3(x / period, 0, z / period)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_left[x + z * size] = lerp(nebulaeColor, image_left[x + z * size], val);

		}
	}

	for (int z = 0; z < size; ++z) {
		for (int x = 0; x < size; ++x) {
			float noise_val = (noise.fBm(Vec3(x / period, size / period, z / period)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_right[x + z * size] = lerp(nebulaeColor, image_right[x + z * size], val);

		}
	}

	for (int z = 0; z < size; ++z) {
		for (int y = 0; y < size; ++y) {
			float noise_val = (noise.fBm(Vec3(0, y / period, z / period)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_front[y + z * size] = lerp(nebulaeColor, image_front[y + z * size], val);

		}
	}

	for (int z = 0; z < size; ++z) {
		for (int y = 0; y < size; ++y) {
			float noise_val = (noise.fBm(Vec3(size / period, y / period, z / period)) + 1.0) / 2.0;
			float val = pow(noise_val + nebulaeDensity, falloff);
			val = (val > 1.0) ? 1.0 : val;
			image_back[y + z * size] = lerp(nebulaeColor, image_back[y + z * size], val);

		}
	}
}

// Saves each face of the cube map
void Skybox::saveimg() {
	MyImage imageBack = MyImage(size, size);
	MyImage imageFront = MyImage(size, size);
	MyImage imageUp = MyImage(size, size);
	MyImage imageDown = MyImage(size, size);
	MyImage imageLeft = MyImage(size, size);
	MyImage imageRight = MyImage(size, size);

	std::reverse(image_up.begin(), image_up.end());

	for (int j = 0; j < size; ++j) {
		for (int i = 0; i < size; ++i) {
			imageBack(size - 1 - i, j) = cv::Vec3b(image_back[j + i * size][0], image_back[j + i * size][1], image_back[j + i * size][2]);
			imageFront(size - 1 - i, size - 1 - j) = cv::Vec3b(image_front[j + i * size][0], image_front[j + i * size][1], image_front[j + i * size][2]);
			imageLeft(size - 1 - i, j) = cv::Vec3b(image_left[j + i * size][0], image_left[j + i * size][1], image_left[j + i * size][2]);
			imageRight(size - 1 - i, size - 1 - j) = cv::Vec3b(image_right[j + i * size][0], image_right[j + i * size][1], image_right[j + i * size][2]);
			imageUp(size - 1 - i, j) = cv::Vec3b(image_up[j + i * size][0], image_up[j + i * size][1], image_up[j + i * size][2]);
			imageDown(size - 1 - i, size - 1 - j) = cv::Vec3b(image_down[j + i * size][0], image_down[j + i * size][1], image_down[j + i * size][2]);
		}

	}

	imageFront.save("./skybox_front.png");
	imageBack.save("./skybox_back.png");
	imageUp.save("./skybox_up.png");
	imageDown.save("./skybox_down.png");
	imageLeft.save("./skybox_left.png");
	imageRight.save("./skybox_right.png");
}

#endif