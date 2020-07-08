===================================================================================

3D Scene Demo 

-----------------------------------

By Justin Cheng

===================================================================================

The scene features a low-poly diorama of a fire dancing fantasy character in the 
middle of a field of butterflies. The following graphical features are implemented:

* Instancing (Repeating an object 10,000x) 
* Post-Processing Bloom
* Post-Processing HDR
* Manual lighting (Lambert Shading)
* Emission Mapping
* Oscilating Glowing elements
* Manual / Automatic Camera Control

The following files are supplied. 
* Main.cpp - Main functions & features
* Camera.h - Responsible for camera object 
* MeshObj.h - Loads an .obj mesh file
* ModelObj.h - Can treat multiple mesh objects as a single model object entity
* UseShader.h - Compile GLSL vertex / fragment shaders.

The Shader folder contains all of the vertex and fragment shaders used.
* Overall Scene: main_vshader.glsl & main_fshader.glsl
* Blur Framebuffer: blur_vshader.glsl & blur_fshader.glsl
* Bloom Framebuffer: bloom_vshader.glsl & bloom_fshader.glsl

===================================================================================
