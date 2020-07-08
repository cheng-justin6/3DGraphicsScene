// =================================================================
//
// blur_fshader.glsl
// -----------------------------------
//
// BLUR FRAGMENT SHADER - take highlighted fragments and blur it
//
// =================================================================

#version 330 core

// Input
in vec2 TexCoords;

// Ouput
out vec4 FragColor;

// Input texture
uniform sampler2D image;
uniform bool horizontal;

// Gaussian blur weights
uniform float weight[5] = float[] (0.22702, 0.19459, 0.12162, 0.05405, 0.01622);

void main() {
	// Get texel
	vec2 tex_offset = 1.0 / textureSize(image, 0); 
	vec3 result = texture(image, TexCoords).rgb * weight[0];
	
	// First blur horizontally (5 times total)
	if(horizontal) {
		for(int i = 1; i < 5; ++i) {
			result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	
	// Then blur vertically (5 times total)
	else {
		for(int i = 1; i < 5; ++i) {
			result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}
	
	// Results
	FragColor = vec4(result, 1.0);
}