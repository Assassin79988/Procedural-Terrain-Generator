#ifndef ICOSPHERE_H_
#define ICOSPHERE_H_

#include <vector>
#include <map>
#include <math.h>
#include <iostream>

#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/Eigen.h>

using namespace OpenGP;

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
float t = 0.0f;

struct Face {
	std::vector<int> vertices;

public:
	Face(int a, int b, int c) {
		vertices = std::vector<int> { a, b, c };
	}
};

class Icosphere {
private:
	std::vector<Vec3> points;
	int index;
	std::vector<Face> faces;
	std::vector<Vec3> vertices;
	std::vector<Vec3> verticesTranslated;
	std::vector<Vec2> uvs;
	std::map<long, int> middlePointIndices;
	std::vector<int> wrapped;

	const float goldenRatio = (1.0f + sqrt(5.0f)) / 2.0f;
	int recursions;
	float radius;
	Vec3 pos;

	int addVertex(Vec3 point);
	int getMiddlePoint(Vec3 point1, Vec3 point2);
	int getMidPointIndex(int indexA, int indexB);
	Vec3 lerp(Vec3 a, Vec3 b, float t);
	void subdivide(int recursions);
	void translate();
	std::vector<int> findWrappedUvcoords();
	void fixWrapedUvs();
public:
	Icosphere(Vec3 pos, float radius, int recursions);

	std::vector<unsigned int> genMesh();
	std::vector<Vec3> getVertices();
	void calcUvs();
	std::vector<Vec2> getUvs();
	std::vector<Vec3> getVertexNormals();
	std::vector<Face> getFaces() { return faces; }
	float getRadius() { return radius; }
	Vec3 getCenter() { return pos; }
};

Icosphere::Icosphere(Vec3 pos, float radius, int recursions) {
	this->recursions = recursions;
	this->radius = radius;
	this->pos = pos;

	middlePointIndices = std::map<long, int>();
	faces = std::vector<Face>();
	vertices = std::vector<Vec3>();

	float sizeFactor = radius / (2 * sin(2 * M_PI / 5));

	vertices.push_back(Vec3(-sizeFactor, sizeFactor * goldenRatio, 0));
	vertices.push_back(Vec3(sizeFactor, sizeFactor * goldenRatio, 0));
	vertices.push_back(Vec3(-sizeFactor, -sizeFactor * goldenRatio, 0));
	vertices.push_back(Vec3(sizeFactor, -sizeFactor * goldenRatio, 0));

	vertices.push_back(Vec3(0, -sizeFactor, sizeFactor * goldenRatio));
	vertices.push_back(Vec3(0, sizeFactor, sizeFactor * goldenRatio));
	vertices.push_back(Vec3(0, -sizeFactor, -sizeFactor * goldenRatio));
	vertices.push_back(Vec3(0, sizeFactor, -sizeFactor * goldenRatio));

	vertices.push_back(Vec3(sizeFactor * goldenRatio, 0, -sizeFactor));
	vertices.push_back(Vec3(sizeFactor * goldenRatio, 0, sizeFactor));
	vertices.push_back(Vec3(-sizeFactor * goldenRatio, 0, -sizeFactor));
	vertices.push_back(Vec3(-sizeFactor * goldenRatio, 0, sizeFactor));

	// Adds 5 polygons around point 0
	faces.push_back(Face(0, 11, 5));
	faces.push_back(Face(0, 5, 1));
	faces.push_back(Face(0, 1, 7));
	faces.push_back(Face(0, 7, 10));
	faces.push_back(Face(0, 10, 11));

	// Adds 5 adjacent polygons
	faces.push_back(Face(1, 5, 9));
	faces.push_back(Face(5, 11, 4));
	faces.push_back(Face(11, 10, 2));
	faces.push_back(Face(10, 7, 6));
	faces.push_back(Face(7, 1, 8));

	// Adds 5 polygons around point 3
	faces.push_back(Face(3, 9, 4));
	faces.push_back(Face(3, 4, 2));
	faces.push_back(Face(3, 2, 6));
	faces.push_back(Face(3, 6, 8));
	faces.push_back(Face(3, 8, 9));

	// Adds 5 adjacent polygons
	faces.push_back(Face(4, 9, 5));
	faces.push_back(Face(2, 4, 11));
	faces.push_back(Face(6, 2, 10));
	faces.push_back(Face(8, 6, 7));
	faces.push_back(Face(9, 8, 1));

	subdivide(recursions);
	calcUvs();
	//fixWrapedUvs();
}

Vec3 Icosphere::lerp(Vec3 a, Vec3 b, float t) {
	return a * t - (t - 1.0) * b;
}

int Icosphere::getMidPointIndex(int indexA, int indexB) {
	int smallIndex = std::min(indexA, indexB);
	int largeIndex = std::max(indexA, indexB);

	long key = (smallIndex << 16) + largeIndex;

	int ret;

	if (middlePointIndices.count(key)) {
		ret =  middlePointIndices.at(key);
	}
	else {
		Vec3 p1 = vertices[indexA];
		Vec3 p2 = vertices[indexB];

		Vec3 _middle = lerp(p1, p2, 0.5f).normalized();

		Vec3 middle = Vec3(_middle[0] * radius, _middle[1] * radius, _middle[2] * radius);

		ret = vertices.size();
		vertices.push_back(middle);

		middlePointIndices.insert(std::pair<long, int>(key, ret));
	}

	return ret;
}

void Icosphere::subdivide(int recursions) {
	for (int i = 0; i < recursions; ++i) {
		std::vector<Face> facesNew;
		for (int j = 0; j < faces.size(); ++j) {
			int a = faces[j].vertices[0];
			int b = faces[j].vertices[1];
			int c = faces[j].vertices[2];

			int ab = getMidPointIndex(a, b);
			int bc = getMidPointIndex(b, c);
			int ca = getMidPointIndex(c, a);

			facesNew.push_back(Face(a, ab, ca));
			facesNew.push_back(Face(b, bc, ab));
			facesNew.push_back(Face(c, ca, bc));
			facesNew.push_back(Face(ab, bc, ca));
		}

		if (facesNew.size() != 0) faces = facesNew;
	}
	translate();
}

std::vector<unsigned int> Icosphere::genMesh() {

	std::vector<unsigned int> indices;

	for (int i = 0; i < faces.size() * 3; i = i + 3) {
		int indexA = faces[i / 3].vertices[0];
		int indexB = faces[i / 3].vertices[1];
		int indexC = faces[i / 3].vertices[2];

		indices.push_back(indexA);
		indices.push_back(indexB);
		indices.push_back(indexC);
	}
	
	return indices;

}

void Icosphere::translate() {
	std::vector<Vec3> verticesTranslated;

	Mat4x4 affineTransform = Mat4x4::Identity();
	affineTransform(0, 3) = pos[0];
	affineTransform(1, 3) = pos[1];
	affineTransform(2, 3) = pos[2];

	for (int i = 0; i < vertices.size(); ++i) {
		Vec4 tempV(vertices[i][0], vertices[i][1], vertices[i][2], 1.0);
		tempV = affineTransform * tempV;

		verticesTranslated.push_back(Vec3(tempV[0], tempV[1], tempV[2]));
	}

	this->verticesTranslated = verticesTranslated;

}

std::vector<Vec3> Icosphere::getVertices() {
	return this->verticesTranslated;
}

std::vector<int> Icosphere::findWrappedUvcoords() {
	std::vector<int> wrappedIndices;

	for (int i = 0; i < faces.size(); ++i) {
		int a = faces[i].vertices[0];
		int b = faces[i].vertices[1];
		int c = faces[i].vertices[2];

		Vec3 uvA = Vec3(uvs[a][0], uvs[a][1], 0);
		Vec3 uvB = Vec3(uvs[b][0], uvs[b][1], 0);
		Vec3 uvC = Vec3(uvs[c][0], uvs[c][1], 0);

		Vec3 uvNormal = (uvB - uvA).cross(uvC - uvA);

		if (uvNormal[2] < 0) {
			wrappedIndices.push_back(i);
		}
	}

	return wrappedIndices;
}

void Icosphere::fixWrapedUvs() {
	/*
	for (int i = 0; i < faces.size(); ++i) {
		int a = faces[i].vertices[0];
		int b = faces[i].vertices[1];
		int c = faces[i].vertices[2];

		float ay = uvs[a][1];
		float by = uvs[b][1];
		float cy = uvs[c][1];
		float ax = uvs[a][0];
		float bx = uvs[b][0];
		float cx = uvs[c][0];

		if (bx - ax >= 0.5 && ay != 1) bx -= 1;
		if (cx - bx > 0.5) cx -= 1;
		if (ax > 0.5 && ax - cx > 0.5 || ax == 1 && cy == 0) ax -= 1;
		if (bx > 0.5 && bx - ax > 0.5) bx -= 1;
		if (ay == 0 || ay == 1) ax = (bx + cx) / 2;
		if (by == 0 || by == 1) bx = (ax + cx) / 2;
		if (cy == 0 || cy == 1) cx = (ax + bx) / 2;

		uvs[a][1] = ay;
		uvs[b][1] = by;
		uvs[c][1] = cy;
		uvs[a][0] = ax;
		uvs[b][0] = bx;
		uvs[c][0] = cx;
	}

	*/
	std::vector<int> wrappedIndices = findWrappedUvcoords();

	int verticeIndex = verticesTranslated.size() - 1;
	std::map<long, int> visted = std::map<long, int>();

	for (int i = 0; i < wrappedIndices.size(); ++i) {
		int a = faces[wrappedIndices[i]].vertices[0];
		int b = faces[wrappedIndices[i]].vertices[1];
		int c = faces[wrappedIndices[i]].vertices[2];

		Vec3 A = verticesTranslated[a];
		Vec3 B = verticesTranslated[b];
		Vec3 C = verticesTranslated[c];

		if (uvs[a][0] < 0.25f) {
			long keyA = a;
			if (visted.count(keyA)) {
				uvs[a][0] += 1;
				verticesTranslated.push_back(A);
				verticeIndex++;
				visted[a] = verticeIndex;
				keyA = verticeIndex;
			}
			a = keyA;
		}

		if (uvs[b][0] < 0.25f) {
			long keyB = b;
			if (visted.count(keyB)) {
				uvs[b][0] += 1;
				verticesTranslated.push_back(B);
				verticeIndex++;
				visted[b] = verticeIndex;
				keyB = verticeIndex;
			}
			b = keyB;
		}

		if (uvs[c][0] < 0.25f) {
			long keyC = a;
			if (visted.count(keyC)) {
				uvs[c][0] += 1;
				verticesTranslated.push_back(C);
				verticeIndex++;
				visted[c] = verticeIndex;
				keyC = verticeIndex;
			}
			c = keyC;
		}

		faces[wrappedIndices[i]].vertices[0] = a;
		faces[wrappedIndices[i]].vertices[1] = b;
		faces[wrappedIndices[i]].vertices[2] = c;
	}
}

void Icosphere::calcUvs() {
	for (int i = 0; i < verticesTranslated.size(); ++i) {
		Vec2 temp;
		Vec3 v = (-verticesTranslated[i]).normalized();
		temp[0] = .5f - atan2(v[2], v[0]) / (2 * M_PI);
		temp[1] = .5f - asin(v[1]) / M_PI;
		//std::cout << "u: " << temp[0] << " v: " << temp[1] << std::endl;
		
		uvs.push_back(temp);
	}
}

std::vector<Vec2> Icosphere::getUvs() {
	return this->uvs;
}

std::vector<Vec3> Icosphere::getVertexNormals() {
	std::vector<Vec3> vnormals;

	for (int i = 0; i < verticesTranslated.size(); ++i) {
		vnormals.push_back((verticesTranslated[i] - pos).normalized());
	}

	return vnormals;
}

#endif