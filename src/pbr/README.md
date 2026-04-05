# PBR Shader — Physically-Based Rendering for LightWave 3D

A combined PBR-lite shader that brings modern material concepts to LightWave 5.x
on AmigaOS. Includes Fresnel reflection, roughness, ambient occlusion, metallic
mode, blurred reflections, and environment sampling in a single plugin.

## Features

### Fresnel Reflection
Schlick's approximation for angle-dependent reflectivity. Edges become more
reflective while centers remain transparent. Independent power controls for
reflection and diffuse curves allow simulation of varied real-world materials.

### Roughness
Perturbs surface normals based on object-space position using a deterministic
hash function. This breaks up perfect mirror reflections and sharp specular
highlights, simulating micro-surface detail without requiring bump maps.

### Ambient Occlusion
Casts rays from each surface point into the surrounding hemisphere to detect
nearby geometry. Areas where rays hit nearby surfaces (corners, crevices) are
darkened, adding depth and contact shadows. Configurable sample count (4/8/16),
radius, and strength.

### Metallic Mode
Switches from dielectric (glass/plastic) behavior to metallic. Metals have
high base reflectivity, near-zero diffuse, and boosted specular — their
appearance is dominated by reflections of the environment.

### Blurred Reflections
Replaces LightWave's single-ray mirror reflections with multi-sample cone
tracing. Casts 4/8/16 rays around the reflection direction, with cone spread
controlled by the Blur Spread setting (independent of roughness). The averaged
result is blended into the surface color, producing soft, spread-out
reflections for frosted, rough, or brushed materials.

### Environment Sampling
Approximates indirect lighting by casting 4/8/16 rays into the hemisphere
around the surface normal. Each sample is cosine-weighted for physically
correct importance sampling. The gathered light is added to the surface
color and slightly boosts luminosity, giving objects a sense of being lit
by their surroundings rather than just direct lights.

## Installation

1. Copy `pbr.p` to your LightWave plugins directory
2. Add these lines to your LW config file:

```
Plugin ShaderHandler PBR pbr.p PBR Shader
Plugin ShaderInterface PBR pbr.p PBR Shader
```

3. Restart LightWave

## Usage

1. Open the **Surfaces** panel in Layout
2. Select a surface and go to the **Shaders** section
3. Add **PBR Shader** from the shader list
4. Click **Options** to adjust settings
5. Render to see the effect

### Settings

| Setting | Range | Default | Description |
|---|---|---|---|
| Index of Refraction | 1.0 – 5.0 | 1.5 | Base reflectivity from IOR |
| Metallic | on/off | off | Switch to metallic material behavior |
| Affect Reflection | on/off | on | Boost mirror at glancing angles |
| Reflection Power | 1 – 10 | 5 | Reflection curve steepness |
| Affect Transparency | on/off | on | Reduce transparency at glancing angles |
| Affect Diffuse | on/off | on | Reduce diffuse at glancing angles |
| Diffuse Power | 1 – 10 | 5 | Diffuse curve steepness |
| Enable Roughness | on/off | off | Perturb normals for rough surfaces |
| Roughness Amount | 0 – 100 | 20 | Intensity of normal perturbation |
| Enable AO | on/off | off | Ray-based ambient occlusion |
| AO Samples | 4/8/16 | 8 | Rays per surface point |
| AO Radius | float | 1.0m | Maximum occlusion distance |
| AO Strength | 0 – 100 | 50 | Occlusion darkening intensity |
| Blurred Reflections | on/off | off | Multi-sample cone-traced reflections |
| Blur Samples | 4/8/16 | 8 | Rays per reflection cone |
| Blur Spread | 0 – 100 | 30 | Cone spread angle (independent of roughness) |
| Environment Lighting | on/off | off | Hemisphere-sampled indirect light |
| Env Samples | 4/8/16 | 8 | Rays per hemisphere sample |
| Env Strength | 0 – 100 | 50 | Indirect lighting intensity |

### Material Presets

**Glass**: IOR 1.5, Reflection on, Transparency on, Diffuse on, Roughness off
- Set surface transparency to 80-100%, mirror to 10-20%

**Water**: IOR 1.33, same as glass but with low roughness (5-10) enabled

**Polished metal**: Metallic on, IOR 2.0+, Roughness off
- Set surface color to metal tint, high mirror

**Brushed metal**: Metallic on, IOR 2.0+, Roughness on (30-50), Blurred Reflections on
- Roughness breaks up reflections, blur softens them realistically

**Plastic**: IOR 1.45, Metallic off, low Roughness (10-20)

**Concrete/stone**: IOR 1.5, Metallic off, high Roughness (60-80), AO on,
Environment Lighting on (30-50) for ambient fill

### Performance Notes

- **Fresnel, Roughness, Metallic**: Very fast — no extra rays, pure math
- **Ambient Occlusion**: Slower — casts 4-16 rays per surface point. Use
  4 samples for previews, 8-16 for final renders
- **Blurred Reflections**: Similar cost to AO — casts 4-16 rays per reflective
  surface point. Only active on surfaces with mirror > 0
- **Environment Sampling**: Casts 4-16 rays per surface point for indirect
  lighting. Combine with AO sparingly — both cast rays and costs stack
- On JIT-equipped emulators ray-based features are quite manageable; on
  real 68k hardware, limit ray-casting features to 4 samples each
- All features can be independently enabled/disabled

## Scene Persistence

All settings are saved with the scene file and restored on reload.
