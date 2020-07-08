// =================================================================
//
// main_vshader.glsl
// -----------------------------------
//
// OBJECT VERTEX SHADER - Push the object's vertex positions,
// normals, and texture coordinates to the fragment shader
//
// =================================================================

#version 330 core

// Inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in mat4 instanceMatrix;

// Outputs
out vec2 TexCoords;
flat out int instanceID;

// Output structure to fragment shader
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

// Input Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Instance 
uniform int instance;

// Main function
void main() {	
	// See if I have to work with either instanced or non-instanced mesh
	if(instance) {
		gl_Position = projection * view  * model * instanceMatrix * vec4(position, 1.0f); 
	}
	else
		gl_Position = projection * view * model * vec4(position, 1.0f);
	
	// Output to fragment shader
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * normal;
    vs_out.TexCoords = texCoords;
	instanceID = gl_InstanceID;
}