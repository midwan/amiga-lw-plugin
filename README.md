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

## Toolchain

Uses `sacredbanana/amiga-compiler:m68k-amigaos` Docker image providing:
- `m68k-amigaos-gcc` 6.5.0b (GCC cross-compiler)
- Full AmigaOS NDK headers and libraries
- libnix (no ixemul.library dependency)

## Building

```bash
./build.sh          # Build SDK library + all plugins
./build.sh objswap  # Build ObjSwap only
./build.sh clean    # Clean build artifacts
```

CI builds run automatically via GitHub Actions using the same Docker image.

## Installation

Copy the `.p` file(s) from `build/` to your LightWave plugins directory
on the Amiga, then add the plugin lines to your LW config file:

```
Plugin ObjReplacementHandler ObjSwap objswap.p ObjSwap
Plugin ObjReplacementInterface ObjSwap objswap.p ObjSwap
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
    └── objswap/          # ObjSwap plugin source
```
