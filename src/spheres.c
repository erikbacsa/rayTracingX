/*
spheres.c

*/

#include <stdlib.h>
#include <stdio.h>
#include "spheres.h"
#include "math.h"

#ifndef INIT_CAP
#define INIT_CAP 5
#endif

/*
Initializes the World structure with default capacity

Allocates memory for the spheres array and initializes all pointers to NULL

Pointer to the World structure to initialize
*/

void worldInit(World *world){
    world->size = 0;
    // (*world).size = 0;

    world->capacity = INIT_CAP;
    // (*world).capacity = INIT_CAP;

    //Each element in the array is a Sphere*, so allocate memory for Sphere pointers
    //NULL each pointer intially until we are ready to use given sphere
    //(*world).spheres = calloc(INIT_CAP, sizeof(Sphere *));
    world->spheres = calloc(INIT_CAP, sizeof(Sphere *));
    // world->spheres = malloc(INIT_CAP * sizeof(Sphere *));
    // for (int i = 0; i < (world->capacity); i++){
    //     world->spheres[i] = NULL;
    //}

    if ((world)->spheres == NULL){
        fprintf(stderr, "Failed to allocate memory for spheres array.\n");   
        world->capacity = 0;
        exit(EXIT_FAILURE);
    }

}

/*
Frees all resources associated with the World structure
*
Frees each Sphere structure and the spheres array itself
*
Pointer to the World structure to free
*/

void freeWorld(World *world){

    //free each pointer to a Sphere structure within array
    int total = world->size;
    // int total = (*world).size;
    for (int i = 0; i<total; i++) {
        //free each pointer in the array
        // free( (*world).spheres[i] );
        free(world->spheres[i]);
    }

    //free the array of pointers
    // free( (*world).spheres);
    free(world->spheres);
    world->spheres = NULL;
    world->size = 0;
    world->capacity = 0;
}



/*
Add spheres to world
*
If the World is at capacity, it resizes the spheres array by doubling its capacity
*
Pointer to the World structure to free if resize fails
*/
void addSphere(World *world, Sphere *sphere){

    //if the max capacity size was hit, increase array size
    if(world->size >= world->capacity){
        int temp_cap = world->capacity * 2;
        //Sphere within world is a pointer to a pointer of Sphere struct
        Sphere **temp = realloc(world->spheres, temp_cap * sizeof(Sphere *));

        if (temp == NULL) {
            freeWorld(world);
            exit(EXIT_FAILURE);
        }
        //if successfully reallocated memory
        world->spheres = temp;
        world->capacity = temp_cap; //update capacity

        //Initialize new sphere pointers to NULL
            for (int i = world->size; i < world->capacity ; i++){
                world->spheres[i] = NULL;
            }
    }

    int temp_pos = world->size;
    world->spheres[temp_pos] = sphere;
    world->size += 1;

}

/*
Create sphere with given parameters
*
Allocates memory for a Sphere and initializes its properties
*
radius is radius of sphere
position is vector position of Sphere
color is color vector of Sphere
*/
Sphere *createSphere(float radius, Vec3 position, Vec3 color){
    
    Sphere *temp_sphere;
    temp_sphere = malloc(sizeof(Sphere));
    if (temp_sphere == NULL){
        fprintf(stderr, "MEMORY ALLOCATION failed for sphere\n");
        exit(EXIT_FAILURE);
    }

    temp_sphere->r = radius;
    temp_sphere->pos = position;
    temp_sphere->color = color;

    return temp_sphere;
}

/*
This function should check if sphere intersects with the ray rayPos + t*rayDir. 
If there is an intersection then it should provide the value of t through the pointer float *t and return 1.
This can then be used to calculate the intersection point and normal vector. 
If there is no intersection then it should return 0.
*
* p = rayPos vector | d = rayDir vector | c = centre position of sphere vector | r = radius of sphere
*
* t^2(d * d) + 2td * (p-c) + (p-c)(p-c) - r^2 = 0
* a = t^2(d * d)
* b = 2td * (p-c)
* c = (p-c)(p-c) - r^2
* discriminant = b^2 - 4ac
*
* quadratic formula: t = [-b +- sqrt(b*b - 4*a*c)] / 2a
*
*/
int doesIntersect(const Sphere *sphere, Vec3 rayPos, Vec3 rayDir, float *t){


    //Get a, b, c for quadratic formula
    Vec3 V = subtract(rayPos, sphere->pos);
    float a = dot(rayDir, rayDir);
    float b = 2.0f * dot(rayDir, V);
    float c = dot(V,V) - (sphere->r * sphere->r);

    float discriminant = (b*b) - (4*a*c);

    //check discriminant
    if (discriminant < 0) //does not intersect(complex root)
        return 0;

    //if (discriminant >= 0); 
    //does intersect either 1 intersections or two
    //solve for t using quadratic formula
    float t1, t2;
    t1 = (-b + sqrt(discriminant)) / (2.0f*a);
    t2 = (-b - sqrt(discriminant)) / (2.0f*a);

    //both t1 and t2 have to be positive
    if (t1 > 0 && t2 > 0){
        *t = t1 < t2 ? t1 : t2;
        return 1;
    } else if (t1 > 0) { // if only 1 if t1 and t2 is positive, circle might have to return 0 since circle is between camera
        *t = t1;
        return 0;
    } else if (t2 > 0) {
        *t = t2;
        return 0;
    }
    return 0;
}


