#ifndef SHADER_H
#define SHADER_H

#include<string>
#include<iostream>
#include<fstream>
#include<glew.h>
#include<glm.hpp>

struct Shader{

	GLuint programID;;
	void LoadShaders(char* vertexFile, char* fragmentFile);

	void UseProgram();

	bool CheckStatus(GLuint, PFNGLGETSHADERIVPROC, PFNGLGETSHADERINFOLOGPROC, GLenum);
	bool CheckShaderStatus(GLuint);
	bool CheckProgramStatus(GLuint);
	
	char* ReadFile(char* fileName);
};

#endif