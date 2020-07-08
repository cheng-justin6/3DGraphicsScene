// ============================================================================
//
// UseShader.h
// -----------------------------------
//
// SHADER HEADER FILE
//
// The shader class handles opening and compiling shaders
//
// ============================================================================

#ifndef SHADER_H
#define SHADER_H

// Standard Includes
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

// OpenGL includes
#include "GL\glew.h"

using namespace std;

// Shader Class
class Shader {
public:
	GLuint Program;
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	void Use();
};

// Constructor that takes both a vertex and fragment shader path
Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
	// Retrieve vertex  & fragment shader code 
	// ------------------------------------------------------
	string vertexCode, fragmentCode;
	ifstream vShaderFile, fShaderFile;
	vShaderFile.exceptions (ifstream::badbit);
	fShaderFile.exceptions (ifstream::badbit);

	try {
		// Open Files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		stringstream vShaderStream, fShaderStream;

		// Read file into stream
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();

		// Convert to string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
	}
	catch (ifstream::failure e) {
		cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
	}
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar * fShaderCode = fragmentCode.c_str();

	// Compile shaders
	// ------------------------------------------------------
	GLuint vertex, fragment;
	GLint success;
	GLchar infoLog[512];

	// Vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	// Fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	// Attach shaders
	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	glLinkProgram(this->Program);

	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// Delete linked shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use() {
	glUseProgram(this->Program);
}

#endif