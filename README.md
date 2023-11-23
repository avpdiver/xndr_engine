# Xndr Engine

Xndr Engine is my personal Vulkan renderer project.

## Low-level rendering backend

The rendering backend focuses entirely on Vulkan.

- Bindless resources
- Dynamic rendering with shader objects
- No pipelines
- No render passes
- Uses transfer queue to upload images/buffers to GPU
- Deferred destruction of Vulkan objects and memory
- Vulkan GLSL for shaders

## Texture formats

- Internally uses only KTX and KTX2 textures
- gltfconv tool converts PNG, JPG, TGA images to KTX2 texture

## Mesh foramts

- Internally uses own mesh format
- gltfconv tool converts GLTF file to internal format

## Compilers

Tested on GCC 13.

## Platforms

- Linux
- Windows

### Third party software

- [Tracy](https://github.com/wolfpld/tracy.git)
- [GLFW](https://github.com/glfw/glfw.git)
- [concurrentqueue](https://github.com/cameron314/concurrentqueue)
- [basisu](https://github.com/BinomialLLC/basis_universal.git)
- [cgltf](https://github.com/jkuhlmann/cgltf.git)
- [meshoptimizer](https://github.com/zeux/meshoptimizer)