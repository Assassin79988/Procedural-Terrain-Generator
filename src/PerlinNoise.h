#ifndef PERLINNOISE_H_
#define PERLINNOISE_H_

#include <cstdlib>
#include <iostream>
#include <random>
#include <math.h> 
#include "Noise.h"

#include <OpenGP/GL/Eigen.h>
#include "OpenGP/GL/Application.h"

using namespace OpenGP;

// Perlin noise
class PerlinNoise : public Noise {
private:
    const int DEFAULT_PERIOD = 512;

    int period;
    static const unsigned int table_size = 512;

    inline void generateGradients(unsigned int seed);
    inline float lerp(float x, float y, float t) const;
    inline float fade(float t) const;
    inline Vec3 gradient(int index) const;

    int P[table_size * 2];
    Vec3 gradients[table_size];
public:
    PerlinNoise(int w, int h, int octaves, float lacunarity, float H, float offset, int period, unsigned int seed);
    PerlinNoise(int w, int h, int octaves, float lacunarity, float H, float offset) : PerlinNoise(w, h, octaves, lacunarity, H, offset, DEFAULT_PERIOD, DEFAULT_SEED) {}
    PerlinNoise(int w, int h, int period) : PerlinNoise(w, h, DEFAULT_OCTAVES, DEFAULT_LACUNARITY, DEFAULT_H, DEFAULT_OFFSET, period, DEFAULT_SEED) {}
    PerlinNoise(int w, int h) : PerlinNoise(w, h, DEFAULT_OCTAVES, DEFAULT_LACUNARITY, DEFAULT_H, DEFAULT_OFFSET, DEFAULT_PERIOD, DEFAULT_SEED) {}
    PerlinNoise() : PerlinNoise(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_OCTAVES, DEFAULT_LACUNARITY, DEFAULT_H, DEFAULT_OFFSET, DEFAULT_PERIOD, DEFAULT_SEED) {}

    /* Getters and Setters */
    void setPeriod(int period) { this->period = period;  };

    int getPeriod() { return this->period; };

    float eval(const Vec3& point) const override;
    float* perlin2D(int noiseType);
    R32FTexture* getNoiseTexture(int noiseType = 0);
    R32FTexture* convertNoiseToTexture(float* noise);

};

PerlinNoise::PerlinNoise(int w, int h, int octaves, float lacunarity, float H, float offset, int period, unsigned int seed) {
    this->width = w;
    this->height = h;
    this->octaves = octaves;
    this->lacunarity = lacunarity;
    this->H = H;
    this->offset = offset;
    this->period = period;
    this->seed = seed;

    computeExponentArray();
    generateGradients(seed);
}

inline void PerlinNoise::generateGradients(unsigned int seed) {
    std::mt19937 generator(seed);
    std::uniform_real_distribution<> distribution;
    auto rand01 = std::bind(distribution, generator);

    for (int i = 0; i < table_size; ++i) {
        float angle = rand01();
        float theta = acos(2 * angle - 1);
        float phi = 2 * angle * M_PI;

        gradients[i] = Vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
        P[i] = i;
    }

    std::uniform_int_distribution<unsigned> distributionInt;
    auto randInt01 = std::bind(distributionInt, generator);

    for (int i = 0; i < table_size; ++i)
        std::swap(P[i], P[randInt01() & (table_size - 1)]);

    for (int i = 0; i < table_size; ++i) {
        P[table_size + i] = P[i];
    }
}

inline float PerlinNoise::lerp(float x, float y, float t) const {
    return x + t * (y - x);
}

inline float PerlinNoise::fade(float t) const {
    // Quintic interpolation curve
    return t * t * t * (t * (t * 6 - 15) + 10);

    // Cubic interpolation curve
    //return t * t * (3.0f - 2.0f * t);
}

inline Vec3 PerlinNoise::gradient(int index) const {

}

float PerlinNoise::eval(const Vec3& point) const {
    int x0 = (int)floor(point[0]) & (table_size - 1);
    int y0 = (int)floor(point[1]) & (table_size - 1);
    int z0 = (int)floor(point[2]) & (table_size - 1);

    int x1 = (x0 + 1) & (table_size - 1);
    int y1 = (y0 + 1) & (table_size - 1);
    int z1 = (z0 + 1) & (table_size - 1);

    float pointX0 = point[0] - floor(point[0]);
    float pointY0 = point[1] - floor(point[1]);
    float pointZ0 = point[2] - floor(point[2]);

    float pointX1 = pointX0 - 1.0f;
    float pointY1 = pointY0 - 1.0f;
    float pointZ1 = pointZ0 - 1.0f;
    
    Vec3 V000(pointX0, pointY0, pointZ0);
    Vec3 V100(pointX1, pointY0, pointZ0);
    Vec3 V010(pointX0, pointY1, pointZ0);
    Vec3 V001(pointX0, pointY0, pointZ1);
    Vec3 V101(pointX1, pointY0, pointZ1);
    Vec3 V110(pointX1, pointY1, pointZ0);
    Vec3 V011(pointX0, pointY1, pointZ1);
    Vec3 V111(pointX1, pointY1, pointZ1);

    Vec3 g000 = gradients[P[P[P[x0] + y0] + z0 ]];
    Vec3 g100 = gradients[P[P[P[x1] + y0] + z0 ]];
    Vec3 g010 = gradients[P[P[P[x0] + y1] + z0 ]];
    Vec3 g001 = gradients[P[P[P[x0] + y0] + z1 ]];
    Vec3 g101 = gradients[P[P[P[x1] + y0] + z1 ]];
    Vec3 g110 = gradients[P[P[P[x1] + y1] + z0 ]];
    Vec3 g011 = gradients[P[P[P[x0] + y1] + z1 ]];
    Vec3 g111 = gradients[P[P[P[x1] + y1] + z1 ]];

    float dotX0Y0Z0 = V000.dot(g000);
    float dotX1Y0Z0 = V100.dot(g100);
    float dotX0Y1Z0 = V010.dot(g010);
    float dotX0Y0Z1 = V001.dot(g001);
    float dotX1Y0Z1 = V101.dot(g101);
    float dotX1Y1Z0 = V110.dot(g110);
    float dotX0Y1Z1 = V011.dot(g011);
    float dotX1Y1Z1 = V111.dot(g111);

    float a = lerp(dotX0Y0Z0, dotX1Y0Z0, fade(pointX0));
    float b = lerp(dotX0Y1Z0, dotX1Y1Z0, fade(pointX0));
    float c = lerp(dotX0Y0Z1, dotX1Y0Z1, fade(pointX0));
    float d = lerp(dotX0Y1Z1, dotX1Y1Z1, fade(pointX0));

    float e = lerp(a, b, fade(pointY0));
    float f = lerp(c, d, fade(pointY0));

    return lerp(e, f, fade(pointZ0));
}

float* PerlinNoise::perlin2D(int noiseType) {

    float* perlin_noise = new float[width * height];

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {

            if (noiseType == 1) perlin_noise[i + j * height] = hybridMultifractal(Vec3((float)i / (float)period, (float)j / (float)period, 0.0f));
            else perlin_noise[i + j * height] = fBm(Vec3((float) i / (float)period, (float)j / (float)period, 0.0f));
        }
    }

    return perlin_noise;
}

R32FTexture* PerlinNoise::getNoiseTexture(int noiseType) {
    R32FTexture* _tex = new R32FTexture();

    float* noise_data = perlin2D(noiseType);
    _tex->upload_raw(width, height, noise_data);

    delete noise_data;

    return _tex;
}

R32FTexture* PerlinNoise::convertNoiseToTexture(float* noise) {
    R32FTexture* _tex = new R32FTexture();

    _tex->upload_raw(width, height, noise);

    return _tex;
}

#endif