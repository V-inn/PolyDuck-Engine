#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <glad/glad.h>
#include <vector>
#include <cmath>

class Sphere {
public:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    Sphere(float radius, int sectorCount, int stackCount);
    void draw() const;
};

class Cylinder {
public:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    Cylinder(float baseRadius, float topRadius, float height, int sectorCount);
    void draw() const;
};

class Plane {
public:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    Plane(float width, float depth, int resX, int resZ);
    void draw() const;
};

#endif