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
- **Extensively commented source code** explaining both implementation details and the underlying math throughout

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

This produces three executables depending on which stage you want to run:

| Executable | Output |
|------------|--------|
| `MS1_assg` | Text output testing vector math against scene data |
| `MS2_assg` | Grayscale PPM render with lighting and shadows |
| `FS_assg`  | Full color PPM render with anti-aliasing |

**2. Render a scene:**
```bash
./FS_assg <input_file> <output_file.ppm>
```

**Example using an included test scene:**
```bash
./FS_assg FS_Testcases/12_input.txt output.ppm
```

Additional sample scenes are available in `FS_Testcases/`, `MS1_Testcases/`, and `MS2_Testcases/`.

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

`ppmcmp.py` applies a Gaussian blur to both images before computing the average per-pixel Euclidean distance in RGB space. This accounts for minor floating-point differences between machines.

| Score | Meaning |
|-------|---------|
| 0 | Identical images |
| < 5 | Passing (acceptable floating-point variance) |
| > 5 | Meaningful visual difference |
| 441.67 | Maximum possible (pure white vs pure black) |

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

**Field reference:**
- `image width / height` — output image resolution in pixels
- `viewport height` — height of the virtual viewport in world units (controls field of view)
- `focal length` — distance from camera to viewport (camera is always at origin)
- `light position + brightness` — single point light source position and intensity
- `colors` — list of HEX colors (e.g. `0xFF0000`), sorted internally before use
- `background color index` — which color from the list to use as background
- `spheres` — each defined by position `(x, y, z)`, radius, and a color index into the sorted list

---

## Project Structure

```
src/
├── assg.c       # Main entry point, rendering pipeline, and anti-aliasing loop
├── assg.h       # Scene, camera, viewport, light, and background color structs
├── vector.c     # Vec3 math library (add, subtract, dot, normalize, length, distance)
├── vector.h
├── spheres.c    # Sphere intersection (quadratic solver), dynamic world array
├── spheres.h
├── color.c      # PPM color writing, HEX to RGB unpacking, color sort comparator
├── color.h
Makefile         # Builds all three targets with correct preprocessor macros
```

---

## How It Works

RayTracerX simulates light by casting rays from a virtual camera through each pixel into the scene. If a ray hits a sphere, the engine calculates lighting, shadows, and color at that point. If it hits nothing, the background color is used.

### 1. Ray Generation

The camera sits at the origin `(0, 0, 0)` and always points along the negative Z axis. A virtual viewport rectangle sits at `z = -focalLength` in front of the camera.

For each pixel `(x, y)`, the engine:
1. Normalizes the pixel coordinates to `[0, 1]`
2. Shifts them to `[-0.5, 0.5]` so the origin is at the image center
3. Scales by the viewport dimensions to get world coordinates
4. Casts a ray from the camera origin through that point

For anti-aliasing (`FS_assg` only), this is done 9 times per pixel using a 3x3 subgrid offset pattern, and the results are averaged.

### 2. Sphere Intersection

For each ray, the engine checks every sphere in the scene by solving:

```
t²(d·d) + 2t(d·(p-c)) + (p-c)·(p-c) - r² = 0
```

Where `p` is the ray origin, `d` is the ray direction, `c` is the sphere center, and `r` is the radius. This is a standard quadratic equation in `t`.

The discriminant `b² - 4ac` determines the result:
- Negative: ray misses the sphere entirely
- Zero or positive: ray hits, solve for `t` using the quadratic formula

The smallest positive `t` across all spheres gives the closest hit.

### 3. Lighting

At the intersection point, intensity is calculated as:

```
I₀ = brightness x max(lightDir · normal, 0) / dist(light, point)²
I  = min(I₀, 1.0)
```

The dot product between the light direction and surface normal gives the angle of incidence. Dividing by distance squared simulates light falloff. The result is clamped to `[0, 1]`.

### 4. Shadow Casting

A secondary ray is cast from the intersection point toward the light source. Before casting, the origin is nudged slightly along the surface normal:

```
shadowOrigin = intersectionPoint + 0.001 * normal
```

This prevents the ray from immediately re-intersecting the same sphere due to floating-point precision. If the shadow ray hits any other sphere before reaching the light, the surface color is multiplied by `0.1`.

### 5. Anti-Aliasing

Each pixel is divided into a 3x3 subgrid. A full render pass (ray generation, intersection, lighting, shadow) is performed at the center of each cell:

```
offsets = {-1/3, 0, +1/3}
```

The 9 resulting color samples are summed and divided by 9 to get the final pixel color.

### 6. Output

Colors are written to a PPM file in P3 (ASCII) format. Each `Vec3` color in `[0, 1]` is converted to integers in `[0, 255]` and written as `R G B` triplets left to right, top to bottom.

---

## Design Decisions

- **Both intersection `t` values must be positive** — `doesIntersect` only returns a hit when both solutions to the quadratic are in front of the camera. This prevents rendering geometry when the camera is inside a sphere and avoids artifacts from behind-camera hits.
- **Dynamic array starts at capacity 5 and doubles on overflow** — keeps `addSphere` amortized O(1) without over-allocating for small scenes.
- **`0.001 * normal` shadow offset** — chosen to be large enough to escape floating-point self-intersection but small enough not to visually displace the shadow origin.
- **Colors are sorted before assignment** — the input provides colors in arbitrary order with index references. The engine sorts the color list using a custom comparator before mapping indices to spheres, ensuring consistent color assignment regardless of input order.

---

## Dependencies
- Standard C library
- `math.h` (link with `-lm`)
