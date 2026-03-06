/*
color.c
*/

#include "color.h"
#include "math.h"


// function to convert packed RGB (e.g., 0xFF00FF) to Vec3 with values in [0,1]
Vec3 unpackRGB(unsigned int packedRGB) {
    Vec3 color;
    // Shortened bits for representation 0xFF == 1111 1111
    //  0xFF00FF == 1111 1111 0000 1111 1111 
    //Move 16 bits left  0000 0000 0000 0000 1111 1111
    // & 0xFF == 1111 1111 & 1111 1111 == 1111 1111
    //RED COMPONENT bits 16-23 | GREEN bits 8-15 | BLUE bits 0-7
    color.x = ((packedRGB >> 16) & 0xFF) / 255.0f; // Red
    color.y = ((packedRGB >> 8) & 0xFF) / 255.0f;  // Green
    color.z = (packedRGB & 0xFF) / 255.0f;         // Blue
    return color;
}

// Clamp a float value between 0 and 1
float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void writeColour(FILE *ppmFile, Vec3 color) {
    // Clamp the color values to [0,1]
    float r = clamp(color.x, 0.0f, 1.0f);
    float g = clamp(color.y, 0.0f, 1.0f);
    float b = clamp(color.z, 0.0f, 1.0f);
    
    // // convert to [0,255]
    int ir = (int)(r * 255.0f);
    int ig = (int)(g * 255.0f);
    int ib = (int)(b * 255.0f);

    // Write to PPM file
    fprintf(ppmFile, "%d %d %d ", ir, ig, ib);
}

int compareColor(const void *a, const void *b)
{
    int a1 = 0, b1 = 0;
    for (size_t i = 0; i < sizeof(int); i++)
    {
        a1 |= (*((unsigned char*)a + i) & 0x0F) << (i * 8);
        b1 |= (*((unsigned char*)b + i) & 0x0F) << (i * 8);
    }
    
    return (a1 < b1) ? -1 : (b1 < a1) ? 1 : (*((int*)a) < *((int*)b)) ? -1 : (*((int*)a) > *((int*)b)) ? 1 : 0;
}



