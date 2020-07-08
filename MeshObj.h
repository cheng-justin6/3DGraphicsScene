// ============================================================================
//
// MeshObj.h
// -----------------------------------
//
// MESH OBJECT HEADER FILE
//
// The Mesh object class is responsible for importing an .obj file along with 
// any respective textures and material files.
//
// ============================================================================

#pragma once

// Standard Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// OpenGL includes
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "UseShader.h"
#include "assimp\Importer.hpp"

using namespace std;
using namespace glm;

// Holds vertex information for location, normals, and texture coords
struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

// Holds path for texture
struct Texture {
	GLuint id;
	string type;
	aiString path;
};

// Mesh class
class Mesh {
private:
	// Buffer objects used when rendering	
	void setupMesh();

public:
	// Data
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	GLuint VAO, VBO, EBO;

	// Functions
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	void Draw(Shader shader);
	void DrawInstance(Shader shader, GLuint num);
};

// Set up the buffer objects 
void Mesh::setupMesh() {
	// Create buffers & arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	// Load vertex information
	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	// Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

	// Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);
}

// Constructor
Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures){
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->setupMesh();
}

// Render the mesh in the window
void Mesh::Draw(Shader shader){
	// Keep track on the number of textures
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;	
	GLuint emissionNr = 1;

	// Loop through and bind all the attached textures
	for(GLint i = 0; i < this->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream ss;
		string number;
		string name = this->textures[i].type;
		if(name == "texture_diffuse")
			ss << diffuseNr++;
		else if(name == "texture_specular")
			ss << specularNr++;
		else if(name == "texture_emission")
			ss << emissionNr++;
		number = ss.str();
		glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	// Default shininess 
	//glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

	// Render the mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Reset to defaults after the configuration has been completed
	for(GLuint i = 0; i < this->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0  + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

// Instanced Version
void Mesh::DrawInstance(Shader shader, GLuint num) {
	// Keep track on the number of textures
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;
	GLuint emissionNr = 1;

	// Loop through and bind all the attached textures
	for (GLint i = 0; i < this->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream ss;
		string number;
		string name = this->textures[i].type;
		if (name == "texture_diffuse")
			ss << diffuseNr++;
		else if (name == "texture_specular")
			ss << specularNr++;
		else if (name == "texture_emission")
			ss << emissionNr++;
		number = ss.str();
		glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	// Default shininess 
	//glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

	// Render the mesh
	glBindVertexArray(this->VAO);
	glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, num);
	glBindVertexArray(0);

	// Reset to defaults after the configuration has been completed
	for (GLuint i = 0; i < this->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}