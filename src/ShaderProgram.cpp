#include "ShaderProgram.h"



rgle::ShaderProgram::ShaderProgram()
{
}

rgle::ShaderProgram::ShaderProgram(std::string name, const char * vertexShader, const char * fragmentShader)
{
	this->id = name;
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
		throw Exception("could not open file: " + std::string(vertexShader), LOGGER_DETAIL_DEFAULT);
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
		throw Exception("could not open file: " + std::string(vertexShader), LOGGER_DETAIL_DEFAULT);
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
		throw Exception(std::string("failed to compile vertex shader: ") + errorMessage.data(), LOGGER_DETAIL_DEFAULT);
	}

	logLength = 0;
	char const* fragPointer = fragCode.c_str();
	glShaderSource(fragmentShaderID, 1, &fragPointer, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetShaderInfoLog(fragmentShaderID, logLength, nullptr, &errorMessage[0]);
		throw Exception(std::string("failed to compile fragment shader: ") + errorMessage.data(), LOGGER_DETAIL_DEFAULT);
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
		throw Exception(std::string("failed to create shader program: ") + errorMessage.data(), LOGGER_DETAIL_DEFAULT);
	}

	glDetachShader(_programID, vertexShaderID);
	glDetachShader(_programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}


rgle::ShaderProgram::~ShaderProgram()
{
}

GLuint rgle::ShaderProgram::programId()
{
	return _programID;
}

void rgle::ShaderProgram::use()
{
	glUseProgram(_programID);
}

std::string & rgle::ShaderProgram::typeName()
{
	return std::string("rgle::ShaderProgram");
}

void checkGLErrors(int line)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "[" << line << "]OpenGL ERROR: (" << err << ")" << glewGetErrorString(err) << std::endl;
	}
}

rgle::ShaderManager::ShaderManager()
{
	this->_shaderPrograms = std::vector<sShaderProgram>();
}

rgle::ShaderManager::ShaderManager(std::initializer_list<sShaderProgram> programs)
{
	this->_shaderPrograms = std::vector<sShaderProgram>();
	for (const sShaderProgram& program : programs) {
		_shaderPrograms.push_back(program);
	}
}

rgle::ShaderManager::~ShaderManager()
{
}

void rgle::ShaderManager::addShader(sShaderProgram shader)
{
	for (int i = 0; i < _shaderPrograms.size(); i++) {
		if (shader->id == _shaderPrograms[i]->id) {
			throw IdentifierException("shader program already exists", shader->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	_shaderPrograms.push_back(shader);
}

rgle::sShaderProgram rgle::ShaderManager::operator[](std::string name)
{
	for (int i = 0; i < this->_shaderPrograms.size(); i++) {
		if (this->_shaderPrograms[i]->id == name) {
			return this->_shaderPrograms[i];
		}
	}
	return nullptr;
}

void APIENTRY rgle::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam)
{
	std::cerr << (type == GL_DEBUG_TYPE_ERROR ? "[GL ERROR]" : "[CALLBACK]")
		<< " type = "
		<< type
		<< ", severity = "
		<< severity
		<< ", message = "
		<< message
		<< ", params = "
		<< userParam
		<< "\n";
}
