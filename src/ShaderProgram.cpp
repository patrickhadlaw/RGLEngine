#include "ShaderProgram.h"



cppogl::ShaderProgram::ShaderProgram()
{
}

cppogl::ShaderProgram::ShaderProgram(std::string name, const char * vertexShader, const char * fragmentShader)
{
	this->_name = name;
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexCode;
	std::ifstream vertexFile(vertexShader, std::ios::in);
	if (vertexFile.is_open()) {
		std::string line = "";
		while (getline(vertexFile, line)) {
			vertexCode += "\n" + line;
		}
		vertexFile.close();
	}
	else {
		throw std::runtime_error("Error: could not open file: " + std::string(vertexShader));
	}
	std::string fragCode;
	std::ifstream fragFile(fragmentShader, std::ios::in);
	if (fragFile.is_open()) {
		std::string line = "";
		while (getline(fragFile, line)) {
			fragCode += "\n" + line;
		}
		fragFile.close();
	}
	else {
		throw std::runtime_error("Error: could not open file: " + std::string(vertexShader));
	}

	GLint result = GL_FALSE;
	int logLength;
	char const* vertexPointer = vertexCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexPointer, nullptr);
	glCompileShader(vertexShaderID);

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetShaderInfoLog(vertexShaderID, logLength, nullptr, &errorMessage[0]);
		std::cout << errorMessage.data() << std::endl;
	}

	logLength = 0;
	char const* fragPointer = fragCode.c_str();
	glShaderSource(fragmentShaderID, 1, &fragPointer, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetShaderInfoLog(vertexShaderID, logLength, nullptr, &errorMessage[0]);
		std::cout << errorMessage.data() << std::endl;
	}

	_programID = glCreateProgram();
	glAttachShader(_programID, vertexShaderID);
	glAttachShader(_programID, fragmentShaderID);
	glLinkProgram(_programID);

	glGetProgramiv(_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetProgramInfoLog(_programID, logLength, nullptr, &errorMessage[0]);
		std::cout << errorMessage.data() << std::endl;
	}

	glDetachShader(_programID, vertexShaderID);
	glDetachShader(_programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}


cppogl::ShaderProgram::~ShaderProgram()
{
}

GLuint cppogl::ShaderProgram::id()
{
	return _programID;
}

void cppogl::ShaderProgram::use()
{
	glUseProgram(_programID);
}

std::string cppogl::ShaderProgram::name()
{
	return _name;
}

void checkGLErrors(int line)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		;
		std::cout << "[" << line << "]OpenGL ERROR: (" << err << ")" << std::endl;
	}
}

cppogl::ShaderManager::ShaderManager()
{
}

cppogl::ShaderManager::ShaderManager(std::initializer_list<sShaderProgram> programs)
{
	this->_shaderPrograms = std::vector<cppogl::sShaderProgram>(programs);
}

cppogl::ShaderManager::~ShaderManager()
{
}

void cppogl::ShaderManager::operator=(std::initializer_list<sShaderProgram> programs)
{
	this->_shaderPrograms = std::vector<cppogl::sShaderProgram>(programs);
}

cppogl::sShaderProgram cppogl::ShaderManager::operator[](std::string name)
{
	for (int i = 0; i < this->_shaderPrograms.size(); i++) {
		if (this->_shaderPrograms[i]->name() == name) {
			return this->_shaderPrograms[i];
		}
	}
	return nullptr;
}
