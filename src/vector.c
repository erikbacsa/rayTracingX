/*
vector.c file
includes vector.h

*/

#include <math.h>
#include "vector.h"


//possibly may need to change
//
Vec3 add(Vec3 v1, Vec3 v2)
{
    Vec3 new_vec;
    new_vec.x = v1.x + v2.x;
    new_vec.y = v1.y + v2.y;
    new_vec.z = v1.z + v2.z;
    return new_vec;
}

//vector subtraction between two vectors, return a new 
//Vec3 type
Vec3 subtract(Vec3 v1, Vec3 v2)
{
    Vec3 new_vec;
    new_vec.x = v1.x - v2.x;
    new_vec.y = v1.y - v2.y;
    new_vec.z = v1.z - v2.z;
    return new_vec;
}

//vector scalar multiply
//scalar and vector values are all floats
Vec3 scalarMultiply(float s, Vec3 v)
{
    Vec3 new_vec;
    new_vec.x = s * v.x;
    new_vec.y = s * v.y;
    new_vec.z = s * v.z;

    return new_vec;
}

//implement vector divide, return Vec3 type
Vec3 scalarDivide(Vec3 v, float d)
{
    Vec3 new_vec;
    new_vec.x = v.x / d;
    new_vec.y = v.y / d;
    new_vec.z = v.z / d;

    return new_vec;
}


//calculate norm of a vector or unit vector
//calculate length of vector
//divide vector by it's 
//unit vector  = vector / length
Vec3 normalize(Vec3 v)
{
    Vec3 new_vec;
    float length_temp = length(v);

    //avoid division by zero
    if (length_temp == 0.0f){
        new_vec.x = 0.0f;
        new_vec.y = 0.0f;
        new_vec.z = 0.0f;
        return new_vec;
    }
    new_vec.x = v.x / length_temp;
    new_vec.y = v.y / length_temp;
    new_vec.z = v.z / length_temp;
    return new_vec;
}

//implement dot product between two vectors
//return its float
//dot product is v1 * v2 = v1*u1 + v2*u2 + v3*u3
float dot(Vec3 v1, Vec3 v2)
{
    float dot_product = 0.0f;
    dot_product += v1.x * v2.x;
    dot_product += v1.y * v2.y;
    dot_product += v1.z * v2.z;
    return dot_product;
}


//Calculate length squared, just don't square root it
float length2(Vec3 v)
{
    float temp_x = v.x * v.x;
    float temp_y = v.y * v.y;
    float temp_z = v.z * v.z;
    float temp_length = temp_x + temp_y + temp_z;
    return temp_length;

}


//calculates the length of the vector(magnitude)
//returns float
// Formula: sqrt((v.x)^2 + (v.y)^2 + (v.z)^2)
float length(Vec3 v)
{
    float temp_x = v.x * v.x;
    float temp_y = v.y * v.y;
    float temp_z = v.z * v.z;
    float temp_length = sqrt(temp_x + temp_y + temp_z);
    return temp_length;
}


//distance squared
float distance2(Vec3 v1, Vec3 v2)
{
    Vec3 diff = subtract(v1, v2);
    return length2(diff);

    // float temp_x = (v1.x - v2.x) * (v1.x - v2.x);
    // float temp_y = (v1.y - v2.y) * (v1.y - v2.y);
    // float temp_z = (v1.z - v2.z) * (v1.z - v2.z);
    // float distance_v = temp_x + temp_y + temp_z;
    // return distance_v;

}

// calculate distance between two vectors
// Formula: sqrt((v1.x - v2.x)^2 + (v1.y - v2.y)^2 + (v1.z - v2.z)^2)
float distance(Vec3 v1, Vec3 v2)
{
    Vec3 diff = subtract(v1, v2);
    return length(diff);

    // float temp_x = (v1.x - v2.x) * (v1.x - v2.x);
    // float temp_y = (v1.y - v2.y) * (v1.y - v2.y);
    // float temp_z = (v1.z - v2.z) * (v1.z - v2.z);
    // float distance_v = sqrt( temp_x + temp_y + temp_z);

    // return distance_v;
}