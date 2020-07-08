// =================================================================
//
// main_fshader.glsl
// -----------------------------------
//
// OBJECT FRAGMENT SHADER - shade the model and scene
//
// =================================================================

#version 330 core

// Input structure from vertex shader.
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;	
} fs_in;

// Output color
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

// Input structure for lights
#define POINT_LIGHTS 2

struct PointLight {
	vec3 lightColor;
	vec3 lightPos;	
	float constant;
	float linear;
	float quadratic;
};

// Input Uniforms
uniform sampler2D diffuseTexture;
uniform sampler2D texture_emission1;
uniform vec3 viewPos;

uniform PointLight pointLights[POINT_LIGHTS];
uniform float emiIntensity;

// Instancing
flat in int instanceID;
uniform int instance;
uniform float particleIntensity1;
uniform float particleIntensity2;
uniform float particleIntensity3;
uniform float particleIntensity4;

// Function Prototypes
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 ViewDir, vec3 color);
vec3 hsv2rgb(vec3 color);

// Main function
void main() {           
    // Obtain basic fragment information
	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 result;
	
	// Apply all point lights and see how it affects the fragments
	for(int i = 0; i < POINT_LIGHTS; i++)
		result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir, color);
	
	// Check if fragment passes the brightness test
	float brightness = dot(result, vec3(0.7126, 0.7152, 0.722));
	if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
	else 
		BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	// End result
    FragColor = vec4(result, 1.0f);
}

// Calculate lighting on object
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color) {
	// Attenuation
	// -------------------------------
	// Affects how far the point lights affect the object
	float dist = length(light.lightPos - fs_in.FragPos);
	float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
	
	// Ambient
	// -------------------------------
	// Slightly red ambient color
	vec3 ambient = vec3(0.2, 0.05, 0.0);
	ambient = ambient * color;
	ambient *= attenuation;
	
    // Diffuse
	// -------------------------------
	// Apply the diffuse map to object
    vec3 lightDir = normalize(light.lightPos - fs_in.FragPos);
    float diff = max(attenuation * dot(lightDir, normalize(normal)), 0.0);
    vec3 diffuse = diff * light.lightColor * color;
	
	// Color the butterflies
	if (instance) {
		diffuse = diffuse + vec3(195.0/255.0, 94.0/255.0, 21.0/255.0);	
		if((instanceID % 4) == 0)
			diffuse = diffuse * vec3(1.0, 1.0* particleIntensity1, 1.0) * vec3(particleIntensity1, particleIntensity1, particleIntensity1);
		else if((instanceID % 4) == 1)
			diffuse = diffuse * vec3(1.0, 1.0* particleIntensity2, 1.0) * vec3(particleIntensity2, particleIntensity2, particleIntensity2);
		else if((instanceID % 4) == 2)
			diffuse = diffuse * vec3(1.0, 1.0* particleIntensity3, 1.0) * vec3(particleIntensity3, particleIntensity3, particleIntensity3);
		else 
			diffuse = diffuse * vec3(1.0, 1.0* particleIntensity4, 1.0) * vec3(particleIntensity4, particleIntensity4, particleIntensity4);
	}
	
	// Emission Mapping
	// -------------------------------
	// Apply emission map to object
	vec3 emission;
	emission = vec3(texture(texture_emission1, fs_in.TexCoords));	
	emission *= emiIntensity;
	
	// Final result
	// -------------------------------
	// Combine all of the values above together
	vec3 lighting = ambient + diffuse + emission; 	
	return lighting;
}

// Convert HSV to RGB
vec3 hsv2rgb(vec3 color)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
    return color.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), color.y);
}

