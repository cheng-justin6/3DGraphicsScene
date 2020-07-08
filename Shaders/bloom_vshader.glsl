// =================================================================
//
// bloom_vshader.glsl
// -----------------------------------
//
// BLOOM VERTEX SHADER - nothing much to do here
//
// =================================================================

#version 330 core

// Inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

// Outputs
out vec2 TexCoords;

void main() {
    // Pass position and texture coordinates
	gl_Position = vec4(position, 1.0f);
    TexCoords = texCoords;
}