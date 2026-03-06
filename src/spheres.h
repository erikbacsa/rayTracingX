//vector.h header file

#ifndef SPHERES_H
#define SPHERES_H

#include "vector.h"

typedef struct {
    //DELETE THIS LATER
    // short unsigned hex; //it's hex color
    // int color_index; //it's color index
    float r;
    Vec3 pos;
    Vec3 color; //this will be from [0,1] for each x, y, z value in vec3
} Sphere;

//size is how many spheres are currently World for the spheres array
//capacity is how many max spheres World can hold
typedef struct {
    Sphere **spheres;
    int size;
    int capacity;
} World;

void worldInit(World *world);
void freeWorld(World *world);
void addSphere(World *world, Sphere *sphere);
Sphere *createSphere(float radius, Vec3 position, Vec3 color);
int doesIntersect(const Sphere *sphere, Vec3 rayPos, Vec3 rayDir, float *t);

#endif
