# Poly
A Vulkan Engine designed from scratch that is developed by me in my spare time.

## Purpose
The general purpose of this project is as a learning exerice for Vulkan, render graph, and for a more advanced engine and project. An ideal goal would be able to create simple games with the engine and to allow for multiple graphics API:s, however these features are not the main focus.

## Currently working on
  - Entity Component System & general fixes/cleanup

## Features (few for the moment)
  - Vulkan API rendering
  - GLSL online compiler
  - Abstracted graphics layer
  - Render Graph (Alpha)
  - ImGui
  
## Currently planned features
  - SPIR-V reflection to automatically generate a more usable interaction with shaders to avoid knowing sets and bindings
  - ImGui implementation with the render graph to allow for a more visual creation of it
  
## Installation
Make sure you have the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

To clone and install the dependencies run the following commands
```git
  git clone https://github.com/Ceanze/Poly.git
  git submodule init
  git submodule update
```

After the project and its submodules have been downloaded run the GenSolution.bat file to generate the project and solution files. The current batfile generates a VS2022 version, to change which type of project files are generated, edit the GenSolution.bat file to use the [appropiate action](https://premake.github.io/docs/Using-Premake). The project has been tested with VS2019 and VS2022.
  
## Code standard
[Here](codestandard.md)
