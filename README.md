# RayTracerX

A high-performance 3D ray tracing engine written in C from scratch. Renders scenes of spheres with physically-based lighting, shadow casting, and anti-aliasing, outputting images in PPM format.

![render example](assets/example.png)

---

## Features

- **Ray-sphere intersection** via quadratic discriminant — correctly rejects behind-camera hits and selects the closest intersecting sphere per ray
- **Physically-based lighting** with distance attenuation: `I = brightness * dot(lightDir, normal) / dist²`, clamped to [0, 1]
- **Shadow casting** via secondary rays, with a `0.001 * normal` origin offset to prevent floating-point self-intersection artifacts
- **3x3 grid supersampling anti-aliasing** — averages 9 stratified samples per pixel to smooth jagged edges
- **Full Vec3 math library** — dot product, normalization, scalar ops, distance, length, all implemented from scratch
- **Custom dynamic array** for scene management with automatic capacity doubling
- **HEX to RGB unpacking** for full-color scene support
- **Multi-milestone Makefile** — compiles three build targets (`MS1_assg`, `MS2_assg`, `FS_assg`) with preprocessor macros

---

## Build

```bash
make
```

This produces three executables:

| Executable | Output |
|------------|--------|
| `MS1_assg` | Text output — tests vector math against scene data |
| `MS2_assg` | Grayscale PPM render |
| `FS_assg`  | Full color PPM render with anti-aliasing |

---

## Usage

```bash
./FS_assg <input_file> <output_file.ppm>
```

**Example:**
```bash
./FS_assg scenes/scene1.txt output.ppm
```

---

## Input Format

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

### Ray Generation
Each pixel maps to a viewport coordinate. A ray is cast from the camera at `(0,0,0)` through the center of each pixel into the scene.

### Intersection
For each ray, the engine solves the quadratic `t²(d·d) + 2t(d·(p-c)) + (p-c)·(p-c) - r² = 0` per sphere. The smallest positive `t` determines the closest hit.

### Lighting
At the intersection point, intensity is calculated as:
```
I₀ = brightness × max(lightDir · normal, 0) / dist(light, point)²
I  = min(I₀, 1.0)
```

### Shadows
A secondary ray is cast from the intersection point toward the light. If it hits any other sphere before reaching the light, the surface is darkened by a factor of `0.1`.

### Anti-Aliasing
Each pixel is sampled at the center of a 3×3 subgrid. The 9 resulting colors are averaged into the final pixel color.

---

## Dependencies

- Standard C library
- `math.h` (link with `-lm`)
