// ============================================================================
//
// ModelObj.h
// -----------------------------------
//
// MODEL OBJECT HEADER FILE
//
// The model class allows several smaller meshes to be grouped up and to be 
// treated as a larger 'Model' object that can handle all of the smaller 
// meshes at once.
//
// ============================================================================


#pragma once

// Standard includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

// OpenGL includes
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "soil\SOIL.h"
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

// Custom headers
#include "MeshObj.h"
#include "UseShader.h"

using namespace std;
using namespace glm;
using namespace Assimp;

// Function prototype
GLint TextureFromFile(const char* path, string directory);

// Model class
class Model {
private:
	// Data
	string directory;
	

	// Functions
	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

public:
	Model();
	Model(GLchar* path);
	void Draw(Shader shader);
	void DrawInstance(Shader shader, GLuint num);

	vector<Mesh> meshes;
	vector<Texture> textures_loaded;
};

// Loads a model and stores the mesh data in seperate mesh classes
void Model::loadModel(string path){
	// Read in file
	Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	// Error Handling
	if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}

	// Retrieve directory of file
	this->directory = path.substr(0, path.find_last_of('/'));

	// Process nodes recursively
	this->processNode(scene->mRootNode, scene);
}

// The model class is structured as a tree of mesh classes, and this function
// will recursively process through all of the mesh nodes.
void Model::processNode(aiNode* node, const aiScene* scene){
	// Process mesh at current node
	for(GLuint i = 0; i < node->mNumMeshes; i++) {
		// Only containts indices
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(mesh, scene));
	}

	// Process the child nodes
	for(GLuint i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], scene);
	}
}

// Take the mesh data and store it in a node within Model class
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene){
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	// Go through each mesh vertices
	for(GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if(mesh->mTextureCoords[0]) {
			vec2 vec;

			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}

	// Go through each face and get indices
	for(GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for(GLuint j = 0; j < face.mNumIndices; j++) 
			indices.push_back(face.mIndices[j]);
	}

	// Process Materials
	if(mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Diffuse Map
		vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// Specular Map
		vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		// Emission Map
		vector<Texture> emissionMaps = this->loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_emission");
		textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

// Check the materials and load textures
vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName){
	// Check material type and load appropriate textures if needed
	vector<Texture> textures;
	for(GLuint i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);

		// Load texture only if it's not loaded before
		GLboolean skip = false;
		for(GLuint j = 0; j < textures_loaded.size(); j++) {
			if(textures_loaded[j].path == str) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if(!skip) {
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str;
			textures.push_back(texture);
			this->textures_loaded.push_back(texture);
		}
	}
	return textures;
}

// Empty Constructor
Model::Model(){
}

// Constructor that loads a model's filepath
Model::Model(GLchar* path){
	this->loadModel(path);
}

// Draw the entire model
void Model::Draw(Shader shader){
	for(GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].Draw(shader);
}

// Draw the entire model (instanced)
void Model::DrawInstance(Shader shader, GLuint num) {
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].DrawInstance(shader, num);
}

// Import textures (not part of Model class)
GLint TextureFromFile(const char* path, string directory){
	// Generate texture ID
	string filename = string(path);
	filename = directory + '/' + filename;
	cout << filename << endl;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);

	// Load texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Initiate texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}