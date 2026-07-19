# AGENTS.md

## Project
C++20 Vulkan + GLFW application using CMake and Ninja. Dependencies are resolved via vcpkg (`VCPKG_ROOT`) and the Vulkan SDK (`VULKAN_SDK`).

## Build
```powershell
.\scripts\build.ps1          # Configure + build (Debug)
.\scripts\build.ps1 -Config Release
```

```bash
cmake -B build -G Ninja
ninja -C build
```

## Run
```powershell
.\scripts\run.ps1            # Build + run
.\scripts\run.ps1 --help     # Pass args to app
```

## Clean
```powershell
.\scripts\clean.ps1
```

## Conventions
- CMake targets should link against `glfw` and the Vulkan library resolved in the root `CMakeLists.txt`.
- Source files live under `src/`.
- Do not commit generated build artifacts or installed vcpkg packages.
- Use C++20 features only.
