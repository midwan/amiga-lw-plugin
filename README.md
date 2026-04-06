# Amiga LightWave Plugin

LightWave 3D 5.x plugins for AmigaOS, cross-compiled with GCC.

## Plugins

### ObjSwap

Object Replacement plugin for Layout. Automatically swaps objects based on
frame number derived from filename suffixes.

Given a base object `Ship.lwo`, the plugin scans the directory for
`Ship_010.lwo`, `Ship_100.lwo` etc. and replaces the object at the
corresponding frames. If no exact frame match exists, the most recent
replacement before the current frame is used.

### Fresnel

Physically-based Fresnel shader for Layout. Adds realistic angle-dependent
reflectivity to surfaces using Schlick's approximation. Edges become more
reflective and less transparent at glancing angles — essential for convincing
glass, water, and polished surfaces. Configurable IOR, power, and independent
control over reflection, diffuse and transparency effects.

### PBR Shader

Combined PBR-lite shader that brings modern material concepts to LightWave 5.x.
Includes Fresnel reflection, roughness (normal perturbation), ambient occlusion
(ray-based hemisphere sampling), metallic mode, blurred reflections (cone-traced
multi-sample rays around reflection direction), and environment sampling (indirect
lighting via hemisphere ray casting with cosine weighting) in a single plugin.
Each feature can be independently enabled and configured.

### SoftShadow

Soft shadow shader that adds penumbra to raytrace shadows. For each surface
point in hard shadow, casts rays toward offset positions around the light
source. Where some rays reach the light and others are blocked, a soft
penumbra edge is produced. Configurable light size, sample count, and
strength. Apply to any surface that receives raytrace shadows.

## Toolchain

Uses `sacredbanana/amiga-compiler:m68k-amigaos` Docker image providing:
- `m68k-amigaos-gcc` 6.5.0b (GCC cross-compiler)
- Full AmigaOS NDK headers and libraries
- libnix (no ixemul.library dependency)

## Building

```bash
./build.sh          # Build SDK library + all plugins
./build.sh objswap  # Build ObjSwap only
./build.sh fresnel  # Build Fresnel only
./build.sh pbr      # Build PBR Shader only
./build.sh softshadow # Build SoftShadow only
./build.sh clean    # Clean build artifacts
```

CI builds run automatically via GitHub Actions using the same Docker image.

## Installation

Copy the `.p` file(s) from `build/` to your LightWave plugins directory
on the Amiga, then add the plugin lines to your LW config file:

```
Plugin ObjReplacementHandler ObjSwap objswap.p ObjSwap
Plugin ObjReplacementInterface ObjSwap objswap.p ObjSwap
Plugin ShaderHandler Fresnel fresnel.p Fresnel
Plugin ShaderInterface Fresnel fresnel.p Fresnel
Plugin ShaderHandler PBR pbr.p PBR Shader
Plugin ShaderInterface PBR pbr.p PBR Shader
Plugin ShaderHandler SoftShadow softshadow.p SoftShadow
Plugin ShaderInterface SoftShadow softshadow.p SoftShadow
```

## SDK

The `sdk/` directory contains the LightWave 5.x SDK headers and support
library, patched for GCC compatibility:

- `sdk/include/` — LW SDK headers (with GCC XCALL_ support added to `plug.h`)
- `sdk/lib/` — Built server library (`server.a`) and startup code (`serv_gcc.o`)
- `sdk/source/` — Server library source, GCC startup assembly, stubs

## Project Structure

```
├── build.sh              # Docker build wrapper
├── Makefile              # Build system
├── VERSION               # Semver version
├── .github/workflows/    # CI
├── sdk/
│   ├── include/          # LW 5.x SDK headers
│   ├── lib/              # Built libraries
│   └── source/           # Library source
└── src/
    ├── objswap/          # ObjSwap plugin source
    ├── fresnel/          # Fresnel shader source
    ├── pbr/              # PBR shader source
    └── softshadow/       # Soft shadow shader source
```
