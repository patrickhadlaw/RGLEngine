#pragma once

#include "Node.h"

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

namespace rgle {

	void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	class Shader {
	public:
		static GLuint compileFile(std::string shaderfile, GLenum type);
		static GLuint compile(std::string shadercode, GLenum type);
	};

	class ShaderProgram : public Node {
	public:
		ShaderProgram();
		ShaderProgram(std::string name, const char* vertexShader, const char* fragmentShader);
		ShaderProgram(std::string name, std::initializer_list<GLuint> shaders);
		~ShaderProgram();

		GLuint programId();
		void use();

		virtual std::string& typeName();

	private:
		GLuint _programID;
	};

	class ShaderManager {
	public:
		ShaderManager();
		ShaderManager(std::initializer_list<std::shared_ptr<ShaderProgram>> programs);
		~ShaderManager();

		void addShader(std::shared_ptr<ShaderProgram> shader);

		std::shared_ptr<ShaderProgram> operator[](std::string name);

	private:
		std::vector<std::shared_ptr<ShaderProgram>> _shaderPrograms;
	};
}