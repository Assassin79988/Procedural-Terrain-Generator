#ifndef NOISE_H_
#define NOISE_H_

#include <cstdlib>
#include <iostream>

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

class Noise {
private:
    float* exponent_array = nullptr;
protected:
    const int DEFAULT_WIDTH = 2048;
    const int DEFAULT_HEIGHT = 2048;
    const float DEFAULT_H = 0.9f;
    const float DEFAULT_LACUNARITY = 2.0f;
    const float DEFAULT_OFFSET = 0.1f;
    const float DEFAULT_OCTAVES = 5;
    const unsigned int DEFAULT_SEED = 2021;

    int width;
    int height;
    int seed;

    // fBm parameters
    float H;
    float lacunarity;
    float offset;
    int octaves;

    // Precompute the exponent array
    void computeExponentArray();
    virtual float eval(const Vec3& point) const = 0;
public:
    /* Getters and Setters */
    void setH(float H);
    void setLacunarity(float lacunarity);
    void setOffset(float offset);
    void setOctaves(int octaves);
    void setWidth(int width);
    void setHeight(int height);

    float getH();
    float getLacunarity();
    float getOffset();
    int getOctaves();
    int getWidth();
    int getHeight();

    float fBm(Vec3 point) const;
    float hybridMultifractal(Vec3 point) const;
};


void Noise::computeExponentArray() {
    if (exponent_array != nullptr) {
        delete[] exponent_array;
    }

    exponent_array = new float[octaves];
    float f = 1.0f;

    for (int i = 0; i < octaves; ++i) {
        exponent_array[i] = std::pow(f, -H);
        f *= lacunarity;
    }
}

float Noise::fBm(Vec3 point) const {
    float val = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        val += eval(point) * exponent_array[i];
        point *= lacunarity;
    }

    return val;
}

float Noise::hybridMultifractal(Vec3 point) const {

    auto noise_func = [this](const Vec3& p) {
        return (1 - abs(eval(p)));
    };

    float val = (noise_func(point) + offset) * exponent_array[0];
    float weight = val;
    point *= lacunarity;

    for (int i = 1; i < octaves; ++i) {
        if (weight > 1.0f) weight = 1.0f;
        float signal = (noise_func(point) + offset) * exponent_array[i];
        val += signal * weight;
        weight *= signal;
        point *= lacunarity;
    }

    return val;
}

#endif