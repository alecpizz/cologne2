# engine

<img width="1606" height="908" alt="image" src="https://github.com/user-attachments/assets/478f26d8-cf26-47b8-8ccf-a53019d85353" />

A small engine/game/renderer/whatever you want to call it. 
## Features:
- ECS
- Voxel Based GI
- Editor
- Model caching
- Physics
- FPS controller
- Ragdolls
- Compute Skinning
- GPU Driven Renderer

## Building
0. Clone the repository. Make sure to use recursive to gather the submodules.
1. Ensure CMake is installed.
2. Create a build directory within the project:
  ```mkdir build``` 
3. Navigate to the build directory:
  ```cd build```
4. Configure Cmake:
  ```cmake ..```
5. Build the project:
  ```cmake --build .```

## GPU
Currently requires a NVIDIA GPU 10 series or greater due to some specific extensions being used. Support for AMD/Intel cards will likely come with a Vulkan rewrite.

## Diary
Progress (8/8):
I've been working on this engine for 3-4 months. I'm at a point where really iterating on gameplay is now possible. I'll need a better level editor, and maybe some greyboxing tools. Just one more refactor...
