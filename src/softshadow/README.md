# SoftShadow — Soft Penumbra for LightWave 3D

Adds soft shadow edges (penumbra) to raytrace shadows in LightWave 5.x on
AmigaOS. Hard shadow boundaries are softened by multi-sampling the light
source as if it had physical size.

## How It Works

For each surface point that LightWave determines is in hard shadow, the
plugin casts multiple rays toward slightly offset positions around the
light. If some rays reach the light while others are blocked by geometry,
the point is in the penumbra zone and receives partial illumination.
Points fully in shadow or fully lit are unchanged.

This is a receiver-side implementation of Percentage Closer Soft Shadows
(PCSS). The penumbra width naturally increases with distance from the
shadow-casting object, matching real-world shadow behavior.

## Installation

1. Copy `softshadow.p` to your LightWave plugins directory
2. Add these lines to your LW config file:

```
Plugin ShaderHandler SoftShadow softshadow.p SoftShadow
Plugin ShaderInterface SoftShadow softshadow.p SoftShadow
```

3. Restart LightWave

## Usage

1. Open the **Surfaces** panel in Layout
2. Select a surface that receives raytrace shadows
3. Add **SoftShadow** from the shader list
4. Click **Options** to adjust settings
5. Enable **Ray Trace Shadows** on your lights
6. Render to see the effect

### Settings

| Setting | Range | Default | Description |
|---|---|---|---|
| Light Size | float | 0.5 | Virtual size of light sources (larger = wider penumbra) |
| Samples | 2/4/8 | 4 | Rays cast per light per surface point |
| Strength | 0 – 100 | 80 | How much light bleeds into shadow edges |

### Tips

- **Light Size** controls penumbra width. Start with 0.5 and increase for
  softer shadows. Values above 2.0 produce very diffuse shadows.
- **Use 2 samples** for previews, 4 for final renders on real 68k hardware.
  8 samples gives smoother results but is slow.
- **Only raytrace shadows** are affected. Shadow map shadows are unaffected.
- **Stackable** with PBR and Fresnel shaders. Apply SoftShadow to any
  surface that needs soft shadow edges.
- **Best results** with scenes that have clear shadow-casting geometry
  (objects between the light and the receiving surface).

### Performance Notes

- Only shadowed surface points are multi-sampled. Fully lit points are
  skipped entirely (single `illuminate()` call, no rayCast).
- Cost scales with: samples × lights × shadowed pixels.
- On JIT-equipped emulators (Amiberry), 4 samples is very manageable.
  On real 68k hardware, use 2 samples.

## How It Differs From Roughness

The PBR shader's roughness perturbs surface normals, which scatters
specular highlights and reflections. SoftShadow works on the shadow
boundary itself — it tests light visibility from multiple angles to
determine how much of the light source is occluded. These are
complementary effects that can be used together.
