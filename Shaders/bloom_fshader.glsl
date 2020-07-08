// =================================================================
//
// bloom_fshader.glsl
// -----------------------------------
//
// BLOOM FRAGMENT SHADER - handle both HDR and Bloom here
//
// =================================================================

#version 330 core

// Input
in vec2 TexCoords;

// Output
out vec4 FragColor;

// Framebuffer textures
uniform sampler2D scene;
uniform sampler2D bloomTex;

// Other uniforms
uniform float exposure;
uniform bool bloom;
uniform bool hdr;

void main() {             
	// Input framebuffer textures for HDR and bloom
	vec3 hdrColor = texture(scene, TexCoords).rgb;      
	vec3 bloomColor = texture(bloomTex, TexCoords).rgb;
	
	// Apply Bloom first
	if(bloom)
	hdrColor += bloomColor; 
	
	// Apply HDR adjustments
	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	
	// Display final combined results
	if(hdr)
	FragColor = vec4(result, 1.0f);
	else 
	FragColor = vec4(hdrColor, 1.0f);
}