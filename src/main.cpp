#include<cstdlib>
#include<cmath>
#include <fstream>

#include <OpenGP/GL/Application.h>

#include "PerlinNoise.h"
#include "Icosphere.h"
#include "Planet.h"
#include "Skybox.h"
#include "Sun.h"

using namespace OpenGP;

// Defines the screen width/height
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// defines the camera sensitivity
#define SENSITIVITY 0.007f

// The radius of the planet
float radius = 50.0f;

// Create two sphere meshes for the sun and planet
Icosphere icosphere(Vec3(0,0,0), radius, 5);
Icosphere sunMesh(Vec3(50, -50, 200), 50, 4);
// Defines the planet, sun and skybox objects
Planet planet = Planet(&icosphere);
Skybox skybox = Skybox(500);
Sun sun = Sun(&sunMesh);

// The camera properties
Vec3 cameraPos;
Vec3 cameraFront;
Vec3 cameraUp;

// properties
float fov;
float speed;
float speedIncrement;
float halflife;

// yaw/pitch for mouse
float yaw;
float pitch;

// Inits the scene
void init() {
	skybox.init();
	planet.init();
	sun.init();
}

// Updates the scene
void update() {
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	skybox.draw(fov, cameraPos, cameraFront, cameraUp);
	glClear(GL_DEPTH_BUFFER_BIT);
	planet.draw(fov, cameraPos, cameraFront, cameraUp);
	sun.draw(fov, cameraPos, cameraFront, cameraUp);
}


int main(int, char**) {
	Application app;

	// Initialize camera position and direction
	cameraPos = Vec3(-68.8, 97.1, -15.9);
	cameraFront = Vec3(0.0f, -1.0f, 0.0f);
	cameraUp = Vec3(0.0f, 0.0f, 1.0f);

	// Initialize FOV and camera speed
	fov = 80.0f;
	speed = 0.5f;
	speedIncrement = 0.01f;

	// Initialize yaw (left/right) and pitch (up/down) angles
	yaw = 0.0f;
	pitch = 0.0f;

	// inits
	init();

	// Listens for applicatio update
	app.add_listener<ApplicationUpdateEvent>([](const ApplicationUpdateEvent&) {
		update();
		});


	// Creates a window
	Window& window = app.create_window([](Window&) { update(); });
	window.set_title("Procedural Terrain");
	window.set_size(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Mouse cords
	Vec2 mouse(0, 0);

	// Listens for mouse movement
	window.add_listener<MouseMoveEvent>([&](const MouseMoveEvent& m) {

		// Camera control
		Vec2 delta = m.position - mouse;
		delta[1] = -delta[1];
		float sensitivity = SENSITIVITY;
		delta = sensitivity * delta;

		yaw += delta[0];
		pitch += delta[1];

		if (pitch > M_PI / 2.0f - 0.01f)  pitch = M_PI / 2.0f - 0.01f;
		if (pitch < -M_PI / 2.0f + 0.01f) pitch = -M_PI / 2.0f + 0.01f;

		Vec3 front(0, 0, 0);
		front[0] = sin(yaw) * cos(pitch);
		front[1] = cos(yaw) * cos(pitch);
		front[2] = sin(pitch);

		cameraFront = front.normalized();
		mouse = m.position;
		});

	// Listens for key input
	window.add_listener<KeyEvent>([&](const KeyEvent& k) {
		if (k.key == GLFW_KEY_W) {
			cameraPos = cameraPos + speed * cameraFront.normalized();
		}

		if (k.key == GLFW_KEY_A) {
			cameraPos = cameraPos - speed * cameraFront.cross(cameraUp).normalized();
		}
		
		if (k.key == GLFW_KEY_D) {
			cameraPos = cameraPos + speed * cameraFront.cross(cameraUp).normalized();
		}
		
		if (k.key == GLFW_KEY_S) {
			cameraPos = cameraPos - speed * cameraFront.normalized();
		}

		if (k.key == GLFW_KEY_UP) {
			fov -= 1.0f;
			if (fov <= 1.0f) fov = 1.0f;
		}

		if (k.key == GLFW_KEY_DOWN) {
			fov += 1.0f;
			if (fov >= 80.0f) fov = 80.0f;
		}

		if (k.key == GLFW_KEY_RIGHT) {
			speed += speedIncrement;
			if (speed >= 1.0f) speed = 1.0f;
		}

		if (k.key == GLFW_KEY_LEFT) {
			speed -= speedIncrement;
			if (speed <= 0.0f) speed = 0.0f;
		}

		});

	return app.run();
}