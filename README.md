# RayTracerX
A high-performance 3D ray tracing engine written in C from scratch. Renders scenes of spheres with physically-based lighting, shadow casting, and anti-aliasing, outputting images in PPM format.

## Example Renders
<img width="1280" height="480" alt="main" src="https://github.com/user-attachments/assets/49276f21-e806-433e-be89-653399c61c7e" />

---

## Features
- **Ray-sphere intersection** via quadratic discriminant, correctly rejecting behind-camera hits and selecting the closest intersecting sphere per ray
- **Physically-based lighting** with distance attenuation: `I = brightness * dot(lightDir, normal) / dist²`, clamped to [0, 1]
- **Shadow casting** via secondary rays with a `0.001 * normal` origin offset to prevent floating-point self-intersection artifacts
- **3x3 grid supersampling anti-aliasing** averaging 9 stratified samples per pixel to smooth jagged edges
- **Full Vec3 math library** including dot product, normalization, scalar ops, distance, and length, all implemented from scratch
- **Custom dynamic array** for scene management with automatic capacity doubling
- **HEX to RGB unpacking** for full-color scene support
- **Multi-milestone Makefile** compiling three build targets (`MS1_assg`, `MS2_assg`, `FS_assg`) with preprocessor macros

---

## Prerequisites
- GCC compiler
- Make
- Python 3 + `numpy` + `scipy` (for testing only)

```bash
pip install numpy scipy
```

---

## Build & Run

**1. Compile:**
```bash
make
```
Produces three executables: `MS1_assg`, `MS2_assg`, `FS_assg`

| Executable | Output |
|------------|--------|
| `MS1_assg` | Text output testing vector math against scene data |
| `MS2_assg` | Grayscale PPM render |
| `FS_assg`  | Full color PPM render with anti-aliasing |

**2. Render a scene:**
```bash
./FS_assg <input_file> <output_file.ppm>
```

**Example using the included test scene:**
```bash
./FS_assg FS_Testcases/12_input.txt output.ppm
```

**3. View the output:**

Open `output.ppm` in any PPM viewer. If you are using VS Code, install the [PPM/PGM/PBM Image Preview](https://marketplace.visualstudio.com/items?itemName=ngtystr.ppm-pgm-viewer-for-vscode) extension.

**4. Clean build files:**
```bash
make clean
```

---

## Testing

Compare your output against the included reference renders:
```bash
python3 ppmcmp.py FS_Testcases/12_output.ppm output.ppm
```
A score of 0 is identical. Anything under 5 is considered passing. The comparison applies a Gaussian blur before diffing to account for minor floating-point differences between machines.

---

## Input Format

Scene files follow this structure:

```
<image width> <image height>
<viewport height>
<focal length>
<light x> <light y> <light z> <brightness>
<number of colors>
<color1 HEX> <color2 HEX> ...
<background color index>
<number of spheres>
<x> <y> <z> <radius> <color index>
...
```

**Example:**
```
640 480
2.0
1.0
20.0 20.0 10.0 1000.0
4
0x1188EE 0xDD2266 0xDD7700 0x11CC66
0
3
2.0 0.0 -5.0 2.0 2
-2.0 0.0 -5.0 2.0 3
0.0 -102.0 -5.0 100.0 1
```

Additional sample scenes are available in `FS_Testcases/`, `MS1_Testcases/`, and `MS2_Testcases/`.

---

## Project Structure

```
src/
├── assg.c       # Main entry point and rendering pipeline
├── assg.h       # Scene, camera, viewport, light structs
├── vector.c     # Vec3 math library
├── vector.h
├── spheres.c    # Sphere intersection, dynamic world array
├── spheres.h
├── color.c      # PPM writing, HEX to RGB unpacking, color sorting
├── color.h
Makefile
```

---

## How It Works

RayTracerX simulates light by casting rays from a virtual camera through each pixel into the scene. If a ray hits a sphere, the engine calculates lighting, shadows, and color at that point. If it hits nothing, the background color is used.

### Ray Generation
Each pixel maps to a viewport coordinate. A ray is cast from the camera at `(0,0,0)` through the center of each pixel into the scene.

### Intersection
For each ray, the engine solves the quadratic equation per sphere:
```
t²(d·d) + 2t(d·(p-c)) + (p-c)·(p-c) - r² = 0
```
The smallest positive `t` determines the closest hit.

### Lighting
At the intersection point, intensity is calculated as:
```
I₀ = brightness x max(lightDir · normal, 0) / dist(light, point)²
I  = min(I₀, 1.0)
```

### Shadows
A secondary ray is cast from the intersection point toward the light. If it hits any other sphere before reaching the light, the surface is darkened by a factor of `0.1`.

### Anti-Aliasing
Each pixel is sampled at the center of a 3x3 subgrid. The 9 resulting colors are averaged into the final pixel color.

---

## Dependencies
- Standard C library
- `math.h` (link with `-lm`)
