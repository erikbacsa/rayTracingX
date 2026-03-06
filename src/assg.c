/*
This will have the main function
*/

#include <stdio.h>
#include <stdlib.h>

#include "vector.h"
#include "assg.h"
#include "spheres.h"
#include "color.h"
#include "math.h"

//GLOBAL VARIABLES//
/* Scene structure
*int imgW;
*int imgH;
*int numColors;
*int numSpheres;
*float aspectRatio;
*/
    Scene scene;
    FILE *input;
    FILE *output;
    Viewport_s viewport;
    Light_s light;
    BackgroundColor_s bgcolor;
    World world;
    unsigned int *hex_arr;
    Camera_s camera = {{0.0f, 0.0f, 0.0f}};

//function prototype to intialize world
int intializeScene();


int main(int argc, char *argv[]) {

    //check if 3 arguements are passed in
    if (argc != 3){
        fprintf(stderr, "Usage: incorrect amount of arguments <%d>\n", argc);
        return EXIT_FAILURE;
    }

    //Open arguement 1 to get file input
    input = fopen(argv[1], "r");
    if (input == NULL){
        fprintf(stderr, "Failed to open: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    //create output FILE
    output = fopen(argv[2], "w");
    if (output == NULL){
        fprintf(stderr, "Failed to open: %s\n", argv[2]);
        fclose(input);
        return EXIT_FAILURE;
    }

    //INTIALIZE WORLD
    if(intializeScene() == 1){
        free(hex_arr);
        return EXIT_FAILURE;
    }

    //Intialize world to store spheres,
    /*  World structure 
    *   Sphere **spheres;   //Sphere array of pointers dynamically allocated
    *   int size;
    *   int capacity;
    */

    /*  Sphere Structure
    *   float r; //radius shpere
    *   Vec3 pos; //position of sphere
    *   Vec3 color; //rgb color of sphere
    */

    //intialize world
    worldInit(&world);




    /*Exicsting variables
    backgroundColor_s bgcolor ------ World world --------- Light_s light
    Vieport_s viewport -------- int scene.imgH, int scene.imgW, ------ unsigned int hexArr[numColors]
    Camera_s camera ------------ int scene.numSpheres ------- int scene.numColors
    */

#ifdef FS

    //qsort the hex_arr, since we're given unsorted hex_values
    qsort(hex_arr, scene.numColors, sizeof(unsigned int), compareColor);

        //create Sphere to use 
        //iterate for each sphere being created
        //*****implement sphereColorIndex later
    for (int i = 0; i<scene.numSpheres; i++){
        //create tempPos for position of sphere
        Vec3 spherePos;
        //tempRad will be temporary radius for sphere
        float sphereRadius;
        //will be for Sphere color indexes
        int sphereColorIndex;

        //if reading fails, exit 
        if (fscanf(input,"%f %f %f %f %d",
        &spherePos.x, &spherePos.y, &spherePos.z, &sphereRadius,
        &sphereColorIndex) != 5) { 

            fprintf(stderr, "Unable to read and get number of spheres\n");
            fclose(input);  
            fclose(output);
            freeWorld(&world);
            return EXIT_FAILURE;
            }

        //Use the sphere's colorindex to upack it's RGB value to values between [0,1]
        Vec3 temp_sphere_color = unpackRGB(hex_arr[sphereColorIndex]);

        //pointer to sphere structure is returned
        Sphere *sphere = createSphere(sphereRadius, spherePos, temp_sphere_color);

        addSphere(&world, sphere);
    }

    // calculate aspect ratio and viewport width
    scene.aspectRatio = (float)scene.imgW / (float)scene.imgH;
    viewport.w = viewport.h * scene.aspectRatio;

    // set background color to 
    Vec3 temp_bgcolor = unpackRGB(hex_arr[bgcolor.color_index]);
    bgcolor.rgb.x = temp_bgcolor.x;
    bgcolor.rgb.y = temp_bgcolor.y;
    bgcolor.rgb.z = temp_bgcolor.z;
    

    // Write PPM header
    fprintf(output, "P3\n%d %d\n255\n", scene.imgW, scene.imgH);

    // Define ambient lighting (optional, can be set to 0 if not required)
    //float ambient = 0.0f; // Set to 0.0f to rely solely on direct lighting

    // Rendering loop: iterate from top to bottom to match PPM's pixel ordering
    //stop just before last pixel ex if 100x100 -> stop at 99x99

    //this loop will center the pixels at the origin, making for example starting point (0,max) == (-0.5, 0.5)
    /* WITHOUT FLIPPING Y in loop this is opposite of how we see, we see images from bottom to top, not top to bottom
    * and left to right
    * (0.0, 0.0) ---------------- (1.0, 0.0)
    *      |                           |
    *      |                           |
    *      |                           |
    * (0.0, 1.0) ---------------- (1.0, 1.0)
    */

    // Map to viewport coordinates  Note: this is how it appears since it is flipped
    /* before shifting by -0.5 in [0,1]     origin (0.5, 0.5) in center ##### at bottom left corner (0,0) #### top right (1,1)
    * (0.0, 1.0) ---------------- (1.0, 1.0)            
    *      |                           |
    *      |                           |
    *      |                           |
    * (0.0, 0.0) ---------------- (1.0, 0.0)
    * 
    * after shifting by -0.5 in range of [-0.5,0.5]    origin (0.0, 0.0) in center ##### at bottom left corner (-0.5, -0.5) ##### top right corner (0.5, 0.5)
    *    (-0.5, 0.5) -------------- (0.5, 0.5)
    *       |                           |
    *       |                           |
    *       |                           |
    *    (-0.5, -0.5) ------------ (0.5, -0.5)
    */

    /* 3x3 grid for anti-aliasing
    +-------------+----------+------------+
    | S(-1/3,-1/3)| S(0,-1/3)| S(1/3,-1/3)|
    +-------------+----------+------------+
    | S(-1/3,0)   | S(0,0)   |   S(1/3,0) |
    +-------------+----------+------------+
    | S(-1/3,1/3) |  S(0,1/3)| S(1/3,-1/3)|
    +------------+-----------+------------+
    */

   // Define the offsets for 3x3 grid sampling
    const int grid_size = 3;
    const float sample_offsets[3] = {-1.0f/((float)grid_size), 0.0f, 1.0f/((float)grid_size)}; // {-1/3, 0, +1/3}

    for (int y = scene.imgH - 1; y >= 0; y--) {
        for (int x = 0; x <= scene.imgW - 1; x++){
            Vec3 totalColor = {0.0f, 0.0f, 0.0f}; //intialized totalColor

            //Iterate over 3x3 grid
            for (int j1 = 0; j1 < grid_size; j1++){
                for (int i1 = 0; i1 < grid_size; i1++){
                    // Compute normalized coordinates (x ranges from [0,1] and y ranges from [0,1])
                    // Normalizing pixels allows for working with any dimension 100x100 or 1920x1080
                    // if imgW and imgH will have resolution, 1920x1080, width of 1920 and height of 1080, changing it to range of [0,1]
                    float nc_x = ((float)x + 0.5f + sample_offsets[i1]) / (float)scene.imgW;
                    float nc_y = ((float)y + 0.5f + sample_offsets[j1]) / (float)scene.imgH;

                    //changing it to pixel representation with (0,0) in center of image
                    //[-0.5, 0.5] X [-0.5, 0.5] --- X by Y values
                    //viewport will have small ranges like for example viewport.w = 3 and viewport.h = 4
                    //converts to viewport value
                    float pixel_x = (nc_x - 0.5f) * viewport.w;
                    float pixel_y = (nc_y - 0.5f) * viewport.h;
                    float pixel_z = -viewport.focal_length;



                    //normalize converts the direction vector to our unit vector(length of 1)
                    //shading equations assume length of 1
                    //subtract(pixelPos, camera.pos) is not necessary because camera is always at (0,0,0) for this
                    Vec3 pixelPos = {pixel_x, pixel_y, pixel_z};
                    Vec3 rayOrigin = camera.pos;
                    Vec3 rayDir = normalize(subtract(pixelPos, rayOrigin));

                    float closest_t = INFINITY; // Initialize to infinity
                    Sphere *closestSphere = NULL;

                    // Find the closest sphere intersected by the primary ray
                    for (int s = 0; s < world.size; s++) {
                        float t;
                        if (doesIntersect(world.spheres[s], rayOrigin, rayDir, &t)) {
                            if (t < closest_t) {
                                closest_t = t;
                                closestSphere = world.spheres[s];
                            }
                        }
                    }

                    Vec3 sampleColor;

                    if (closestSphere == NULL) {
                        //If no intersection; use background color
                        sampleColor = bgcolor.rgb;
                    }

                    else {
                        sampleColor = closestSphere->color;
                        // Parametric equation of a ray P(t) = rayOrigin + t*rayDir
                        // closest_t is the t
                        // Calculate intersection point
                        Vec3 intersectionPoint = add(rayOrigin, scalarMultiply(closest_t, rayDir));

                        // Calculate normal at intersection and normalize
                        // Will get intersection point from origin to surface of sphere
                        // and get distance between centre of sphere to the intersection point | vector2 - vector1 = distance
                        Vec3 normal = subtract(intersectionPoint, closestSphere->pos);
                        // normal vector with distance of 1
                        normal = normalize(normal);
                        

                        // Calculate light direction (from intersection point to light) and normalize
                        Vec3 lightDir = subtract(light.pos, intersectionPoint);
                        float distanceToLight = length(lightDir);
                        lightDir = normalize(lightDir);
                        

                        // Calculate dot product between light direction and normal, clamped to 0
                        // dot product will be the cosine angle between two vectors ---> thats why when two perp angles have dot product it will be 0
                        // if the dot product is negative, then angle between lightDir and normal is greater than 90 degrees
                        //max (dotProduct, 0)
                        //get the bigger of the two values,
                        float dotProduct = dot(lightDir, normal);
                        if (dotProduct < 0.0f)
                            dotProduct = 0.0f;


                        /*
                        Calculate initial intensity
                        * I0 = b* [max(d*n,0) / dist(l,p)^2]
                        l = light position | b = light brightness | p = intersection point | n = surface normal
                        */
                        float I0 = (light.brightness * dotProduct) / (distanceToLight * distanceToLight);

                        // Clamp intensity to a maximum of 1
                        //min either of I0 or 1.0f whichever is smaller
                        float intensity = I0 < 1.0f ? I0 : 1.0f;

                        // Cast shadow ray
                        /*
                        * When casting a ray to check for shadows, make sure that the ray's position vector is not equal to the intersection point exactly. 
                        * It should be slightly moved in front
                        * (this is to avoid floating point precision causing the ray to begin inside the sphere).
                        * p = i + 0.001n | i = intersection point, n = surface normal at the point | p = vector p
                        */
                    //shadow origin is slighly offset toward direction of normal from center of sphere
                        Vec3 shadowOrigin = add(intersectionPoint, scalarMultiply(0.001, normal)); // Offset to prevent self-intersection
                        //Vec3 shadowDir = lightDir; // Direction towards light

                        int inShadow = 0;
                        for (int i = 0; i < world.size; i++) {
                            Sphere *shadowSphere = world.spheres[i];
                            if (shadowSphere == closestSphere)
                                continue; // Skip the sphere itself

                            float t_shadow;
                            //compare with each sphere in world
                            //shadowOrigin is point on sphere we're checking if intersects any sphere toward light
                            //lightDir is direction from light to shadowOrigin
                            if (doesIntersect(shadowSphere, shadowOrigin, lightDir, &t_shadow)) {
                                if (t_shadow > 0.0f && t_shadow < distanceToLight) {
                                    inShadow = 1;
                                    break;
                                }
                            }
                        }

                        if (inShadow) {
                            // Apply shadow factor, will make object darker
                            //object will only have 10% of the light
                            intensity *= 0.1f; // shadowFactor = 0.1
                        }

                        //The final color in RGB, given by should be calculated as follows:
                        //P(x,y) = l*S  l = light position, S = color of the sphere
                        //intensity will scalarMultiply to determine final color
                        sampleColor = scalarMultiply(intensity, sampleColor);
                    }
                    totalColor = add(totalColor, sampleColor);
            }
        }
        totalColor = scalarDivide(totalColor, (float)(grid_size * grid_size));

        // Write color to PPM file
        writeColour(output, totalColor);
    }
    fprintf(output, "\n"); // Newline after each row
}


#elif defined(MS2)

        //create Sphere to use 
        //iterate for each sphere being created
        //*****implement sphereColorIndex later
    for (int i = 0; i<scene.numSpheres; i++){
        //create tempPos for position of sphere
        Vec3 spherePos;
        //tempRad will be temporary radius for sphere
        float sphereRadius;
        //will be for Sphere color indexes
        int sphereColorIndex;

        //if reading fails, exit 
        if (fscanf(input,"%f %f %f %f %d",
        &spherePos.x, &spherePos.y, &spherePos.z, &sphereRadius,
        &sphereColorIndex) != 5) { 

            fprintf(stderr, "Unable to read and get number of spheres\n");
            fclose(input);  
            fclose(output);
            freeWorld(&world);
            return EXIT_FAILURE;
            }

        //for each sphere has default color white
        Vec3 temp_sphere_color = {1.0f, 1.0f, 1.0f};
        //pointer to sphere structure is returned
        Sphere *sphere = createSphere(sphereRadius, spherePos, temp_sphere_color);

        addSphere(&world, sphere);
    }
    // calculate aspect ratio and viewport width
    scene.aspectRatio = (float)scene.imgW / (float)scene.imgH;
    viewport.w = viewport.h * scene.aspectRatio;

    // set background color to black
    bgcolor.rgb.x = 0.0f;
    bgcolor.rgb.y = 0.0f;
    bgcolor.rgb.z = 0.0f;
    

    // Write PPM header
    fprintf(output, "P3\n%d %d\n255\n", scene.imgW, scene.imgH);

    // Define ambient lighting (optional, can be set to 0 if not required)
    //float ambient = 0.0f; // Set to 0.0f to rely solely on direct lighting

    // Rendering loop: iterate from top to bottom to match PPM's pixel ordering
    //stop just before last pixel ex if 100x100 -> stop at 99x99
    //this loop will center the pixels at the origin, making for example starting point (0,max) == (-0.5, 0.5)
    for (int y = scene.imgH - 1; y >= 0; y--) {
        for (int x = 0; x <= scene.imgW - 1; x++){
            // Compute normalized coordinates (x ranges from [0,1] and y ranges from [0,1])
            // Normalizing pixels allows for working with any dimension 100x100 or 1920x1080
            // if imgW and imgH will have resolution, 1920x1080, width of 1920 and height of 1080, changing it to range of [0,1]
            float nc_x = (x + 0.5f) / (float)scene.imgW;
            float nc_y = (y + 0.5f) / (float)scene.imgH;

            /* WITHOUT FLIPPING Y in loop this is opposite of how we see, we see images from bottom to top, not top to bottom
            * and left to right
            * (0.0, 0.0) ---------------- (1.0, 0.0)
            *      |                           |
            *      |                           |
            *      |                           |
            * (0.0, 1.0) ---------------- (1.0, 1.0)
            */

            // Map to viewport coordinates  Note: this is how it appears since it is flipped
            /* before shifting by -0.5 in [0,1]     origin (0.5, 0.5) in center ##### at bottom left corner (0,0) #### top right (1,1)
            * (0.0, 1.0) ---------------- (1.0, 1.0)            
            *      |                           |
            *      |                           |
            *      |                           |
            * (0.0, 0.0) ---------------- (1.0, 0.0)
            * 
            * after shifting by -0.5 in range of [-0.5,0.5]    origin (0.0, 0.0) in center ##### at bottom left corner (-0.5, -0.5) ##### top right corner (0.5, 0.5)
            *    (-0.5, 0.5) -------------- (0.5, 0.5)
            *       |                           |
            *       |                           |
            *       |                           |
            *    (-0.5, -0.5) ------------ (0.5, -0.5)
            */
            //changing it to pixel representation with (0,0) in center of image
            //viewport will have small ranges like for example viewport.w = 3 and viewport.h = 4
            float pixel_x = (nc_x - 0.5f) * viewport.w;
            float pixel_y = (nc_y - 0.5f) * viewport.h;
            float pixel_z = -viewport.focal_length;

            //converts to viewport value

            //normalize converts the direction vector to our unit vector(length of 1)
            //shading equations assume length of 1
            //subtract(pixelPos, camera.pos) is not necessary because camera is always at (0,0,0) for this
            Vec3 pixelPos = {pixel_x, pixel_y, pixel_z};
            Vec3 rayOrigin = camera.pos;
            Vec3 rayDir = normalize(subtract(pixelPos, rayOrigin));

            float closest_t = INFINITY; // Initialize to infinity
            Sphere *closestSphere = NULL;

            // Find the closest sphere intersected by the primary ray
            for (int i = 0; i < world.size; i++) {
                float t;
                if (doesIntersect(world.spheres[i], rayOrigin, rayDir, &t)) {
                    if (t < closest_t) {
                        closest_t = t;
                        closestSphere = world.spheres[i];
                    }
                }
            }

            Vec3 finalColor;
            if (closestSphere == NULL) {
                //If no intersection; use background color
                finalColor = bgcolor.rgb;
            }
            else {
                // Parametric equation of a ray P(t) = rayOrigin + t*rayDir
                // closest_t is the t
                // Calculate intersection point
                Vec3 intersectionPoint = add(rayOrigin, scalarMultiply(closest_t, rayDir));

                // Calculate normal at intersection and normalize
                // Will get intersection point from origin to surface of sphere
                // and get distance between centre of sphere to the intersection point | vector2 - vector1 = distance
                Vec3 normal = subtract(intersectionPoint, closestSphere->pos);
                // normal vector with distance of 1
                normal = normalize(normal);
                

                // Calculate light direction (from intersection point to light) and normalize
                Vec3 lightDir = subtract(light.pos, intersectionPoint);
                float distanceToLight = length(lightDir);
                lightDir = normalize(lightDir);
                

                // Calculate dot product between light direction and normal, clamped to 0
                // dot product will be the cosine angle between two vectors ---> thats why when two perp angles have dot product it will be 0
                // if the dot product is negative, then angle between lightDir and normal is greater than 90 degrees
                //max (dotProduct, 0)
                //get the bigger of the two values,
                float dotProduct = dot(lightDir, normal);
                if (dotProduct < 0.0f)
                    dotProduct = 0.0f;

                // Calculate initial intensity
                /*
                * I0 = b* [max(d*n,0) / dist(l,p)^2]
                l = light position | b = light brightness | p = intersection point | n = surface normal

                */
                float I0 = (light.brightness * dotProduct) / (distanceToLight * distanceToLight);

                // Clamp intensity to a maximum of 1
                //min either of I0 or 1.0f whichever is smaller
                float intensity = I0 < 1.0f ? I0 : 1.0f;

                // Cast shadow ray
                /*
                * When casting a ray to check for shadows, make sure that the ray's position vector is not equal to the intersection point exactly. 
                * It should be slightly moved in front
                * (this is to avoid floating point precision causing the ray to begin inside the sphere).
                * p = i + 0.001n | i = intersection point, n = surface normal at the point | p = vector p
                */
               //shadow origin is slighly offset toward direction of normal from center of sphere
                Vec3 shadowOrigin = add(intersectionPoint, scalarMultiply(0.001, normal)); // Offset to prevent self-intersection
                //Vec3 shadowDir = lightDir; // Direction towards light

                int inShadow = 0;
                for (int i = 0; i < world.size; i++) {
                    Sphere *shadowSphere = world.spheres[i];
                    if (shadowSphere == closestSphere)
                        continue; // Skip the sphere itself

                    float t_shadow;
                    //compare with each sphere in world
                    //shadowOrigin is point on sphere we're checking if intersects any sphere toward light
                    //lightDir is direction from light to shadowOrigin
                    if (doesIntersect(shadowSphere, shadowOrigin, lightDir, &t_shadow)) {
                        if (t_shadow > 0.0f && t_shadow < distanceToLight) {
                            inShadow = 1;
                            break;
                        }
                    }
                }

                if (inShadow) {
                    // Apply shadow factor, will make object darker
                    //object will only have 10% of the light
                    intensity *= 0.1f; // shadowFactor = 0.1
                }

                //The final color in RGB, given by should be calculated as follows:
                //P(x,y) = l*S  l = light position, S = color of the sphere
                // Calculate final grayscale color
                float grayscale = intensity;
                finalColor = (Vec3){grayscale, grayscale, grayscale};
            }

            // Write color to PPM file
            writeColour(output, finalColor);
        }
        fprintf(output, "\n"); // Newline after each row
    }






#elif defined(MS1)
    ///////////PART1 MILESTONE 1 PRINTOUT////////////////////////
    /*
    <background color (Vec3)> + <light position (Vec3)> = <result (Vec3)>
    <background color (Vec3)> - <light position (Vec3)> = <result (Vec3)>
    <viewport width (float)> * <light position (Vec3)> = <result (Vec3)>
    normalize<light position (Vec3)> = <result (Vec3)>
    */

           //create Sphere to use 
        //iterate for each sphere being created
        //*****implement sphereColorIndex later
    for (int i = 0; i<scene.numSpheres; i++){
        //create tempPos for position of sphere
        Vec3 spherePos;
        //tempRad will be temporary radius for sphere
        float sphereRadius;
        //will be for Sphere color indexes
        int sphereColorIndex;

        //if reading fails, exit 
        if (fscanf(input,"%f %f %f %f %d",
        &spherePos.x, &spherePos.y, &spherePos.z, &sphereRadius,
        &sphereColorIndex) != 5) { 

            fprintf(stderr, "Unable to read and get number of spheres\n");
            fclose(input);  
            fclose(output);
            freeWorld(&world);
            return EXIT_FAILURE;
            }

        //for each sphere has default color white
        Vec3 temp_sphere_color = {1.0f, 1.0f, 1.0f};
        //pointer to sphere structure is returned
        Sphere *sphere = createSphere(sphereRadius, spherePos, temp_sphere_color);

        addSphere(&world, sphere);
    }
    
    scene.aspectRatio = (float)scene.imgW / (float)scene.imgH;
    viewport.w = viewport.h * scene.aspectRatio;

    // set background color to black
    bgcolor.rgb.x = 0.0f;
    bgcolor.rgb.y = 0.0f;
    bgcolor.rgb.z = 0.0f;

    Vec3 addResult = add(bgcolor.rgb, light.pos);
    Vec3 subtractResult = subtract(bgcolor.rgb, light.pos);
    Vec3 scalarResult = scalarMultiply(viewport.w, light.pos);
    Vec3 normalizeResult = normalize(light.pos);
    fprintf(output, "(%.1f, %.1f, %.1f) + (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n",
                    bgcolor.rgb.x, bgcolor.rgb.y, bgcolor.rgb.z,
                    light.pos.x, light.pos.y, light.pos.z,
                    addResult.x, addResult.y, addResult.z);

    fprintf(output, "(%.1f, %.1f, %.1f) - (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n",
                    bgcolor.rgb.x, bgcolor.rgb.y, bgcolor.rgb.z,
                    light.pos.x, light.pos.y, light.pos.z,
                    subtractResult.x, subtractResult.y, subtractResult.z);

    fprintf(output, "%.1f * (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n",
                viewport.w,
                light.pos.x, light.pos.y, light.pos.z,
                scalarResult.x, scalarResult.y, scalarResult.z);
    fprintf(output, "normalize(%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n",
                light.pos.x, light.pos.y, light.pos.z,
                normalizeResult.x, normalizeResult.y, normalizeResult.z);
        
    ///////////PART2 MILESTONE 1 PRINTOUT////////////////////////        
    /*
    *   <sphere color (Vec3)> / <sphere radius (float)> = <result (Vec3)>
    *   dot(<light position (Vec3)>, <sphere position (Vec3)>) = <result (float)>
    *   distance(<light position (Vec3)>, <sphere position (Vec3)>) = <result (float)>
    *   length<sphere position (Vec3)> = <result (float)> 
    */
    for(int i = 0; i < world.size; i++){
        Sphere sphere_t = *(world.spheres[i]);
        //can also use (*world.spheres[i]).color
        //Vec3 divideResult = scalarDivide(world.spheres[i]->color, world.spheres[i]->r);
        Vec3 divideResult = scalarDivide(sphere_t.color, sphere_t.r);
        float dotResult = dot(light.pos, sphere_t.pos);
        float distanceResult = distance(light.pos, sphere_t.pos);
        float lengthResult = length(sphere_t.pos);

        fprintf(output, "\n");

        fprintf(output, "(%.1f, %.1f, %.1f) / %.1f = (%.1f, %.1f, %.1f)\n",
                        sphere_t.color.x, sphere_t.color.y, sphere_t.color.z,
                        sphere_t.r, divideResult.x, divideResult.y, divideResult.z);
        fprintf(output, "dot((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n",
                        light.pos.x, light.pos.y, light.pos.z,
                        sphere_t.pos.x, sphere_t.pos.y, sphere_t.pos.z,
                        dotResult);
        fprintf(output, "distance((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n",
                        light.pos.x, light.pos.y, light.pos.z,
                        sphere_t.pos.x, sphere_t.pos.y, sphere_t.pos.z,
                        distanceResult);
        fprintf(output, "length(%.1f, %.1f, %.1f) = %.1f\n",
                        sphere_t.pos.x, sphere_t.pos.y, sphere_t.pos.z,
                        lengthResult);

    
    }
#endif
    
    fclose(input);
    fclose(output);
    freeWorld(&world);
    free(hex_arr);

    return 0;
}



int intializeScene(){
    
    //declare camera
    //Camera_s camera = {0.0f, 0.0f, 0.0f};
    //get and store image width and height into scene

    if (fscanf(input, "%d %d", &scene.imgW, &scene.imgH) != 2){
        fprintf(stderr, "Unable to read imgW & imgH\n");
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    //STORE Viewport height(float), and focal length(float)
    if (fscanf(input, "%f", &viewport.h) != 1){
        fprintf(stderr, "Unable to read viewport height\n");
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    //viewport focal length(float)
    if (fscanf(input, "%f", &viewport.focal_length) != 1){
        fprintf(stderr, "Unable to read viewport focal length\n");
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    //light position (3floats) light brightness(float)
    if (fscanf(input, "%f %f %f %f",
                            &light.pos.x,
                            &light.pos.y,
                            &light.pos.z,
                            &light.brightness) != 4){
    fprintf(stderr, "Unable to read light position or brightness\n");
    fclose(input);
    fclose(output);
    return EXIT_FAILURE;
    }

    //store numColors into scene.numColors
    if (fscanf(input, "%d", &scene.numColors) != 1){
    fprintf(stderr, "Unable to read and get number of colors\n");
    fclose(input);  
    fclose(output);
    return EXIT_FAILURE;
    }

    //create vec array of pointers for the hex colors, of size numColors
    //DELETE if hex colors never changes, use VLAs instead
    // unsigned int *hex_arr = malloc(m * sizeof(unsigned int));
    // if (hex_arr == NULL) {
    //     perror("Failed to allocate memory for colors");
    //     fclose(input);
    //     fclose(output);
    //     return EXIT_FAILURE;
    // }

    //free(hex_arr); IF hex_arr needs to be dynamic, add this to every fail safe
    hex_arr = calloc(scene.numColors, sizeof(unsigned int));
    //read in every hex value
    for (int i = 0; i<scene.numColors; i++){
        if (fscanf(input, "%x", (hex_arr+i)) != 1){
            fprintf(stderr, "Failed to read hexcolor %d\n", *(hex_arr+i));
            fclose(input);  
            fclose(output);
            return EXIT_FAILURE;
        }
    }

    //background color index
    if (fscanf(input, "%d", &bgcolor.color_index) != 1){
        fprintf(stderr, "Unable to read and get background color index\n");
        fclose(input);  
        fclose(output);
        return EXIT_FAILURE;
    }
    //intialize to rgb 0,0,0 by default
    bgcolor.rgb.x = 0.0f;
    bgcolor.rgb.y = 0.0f;
    bgcolor.rgb.z = 0.0f;

    //store numSpheres into scene
    if (fscanf(input, "%d", &scene.numSpheres) != 1){
        fprintf(stderr, "Unable to read and get number of spheres\n");
        fclose(input);  
        fclose(output);
        return EXIT_FAILURE;
    }

return 0;
}