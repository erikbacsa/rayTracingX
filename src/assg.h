//assg.c header file

#ifndef ASSG_H
#define ASSG_H

//include necessary libraries
// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include "vector.h"


//camera will always have (0,0,0) position

typedef struct {
    Vec3 pos;
}Camera_s;

//will have height given through input
//will have width calculated to match image aspect ratio
////The camera should always point toward the negative z axis, meaning
//meaning viewport's z value should be -focalLength
typedef struct{
    float w;
    float h;
    float focal_length;

}Viewport_s;

///////////////////LIGHT//////////////
//light source will have position and brightness
//light's position and brightness given through input

typedef struct{
    //pos will be 3 consective float point values
    //will represent (x,y,z) from Vec3
    Vec3 pos;
    float brightness;
}Light_s;

///////////////////background COLOR //////////////
//RGB color in [0,1]
//background color given through input

typedef struct{
    Vec3 rgb;
    int color_index;

}BackgroundColor_s;

typedef struct{
    int imgW;
    int imgH;
    int numColors;
    int numSpheres;
    float aspectRatio;
}Scene;


#endif //ASSG_H