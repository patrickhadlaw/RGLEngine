#pragma once

#include "Exception.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <ctime>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <array>
#include <memory>
#include <map>
#include <assert.h>

#include <GL\glew.h>
#include <GL\GL.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <ft2build.h>
#include <freetype\ftglyph.h>
#include FT_FREETYPE_H

#ifdef NDEBUG
#define DEBUG_MESSAGE
#else
#define DEBUG_MESSAGE std::cout << "[" << __FUNCTION__ << "][" << __LINE__ << "]\n";
#endif

void checkGLErrors(int line);

namespace cppogl {

	class ShaderProgram
	{
	public:
		ShaderProgram();
		ShaderProgram(std::string name, const char* vertexShader, const char* fragmentShader);
		~ShaderProgram();

		GLuint id();
		void use();

		std::string name();

	private:
		std::string _name;
		GLuint _programID;
	};
	typedef std::shared_ptr<ShaderProgram> sShaderProgram;

	class ShaderManager {
	public:
		ShaderManager();
		ShaderManager(std::initializer_list<sShaderProgram> programs);
		~ShaderManager();

		void operator=(std::initializer_list<sShaderProgram> programs);
		sShaderProgram operator[](std::string name);

	private:
		std::vector<sShaderProgram> _shaderPrograms;
	};
}