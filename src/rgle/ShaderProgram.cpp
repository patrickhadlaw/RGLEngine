#include "rgle/ShaderProgram.h"


GLuint rgle::Shader::compileFile(std::string shaderfile, GLenum type)
{
	RGLE_DEBUG_ONLY(rgle::Logger::debug("loading shader file: " + shaderfile, LOGGER_DETAIL_DEFAULT);)
	std::string file;
	std::ifstream ifstream(shaderfile, std::ios::in);
	if (ifstream.is_open()) {
		std::string line = "";
		while (getline(ifstream, line)) {
			file += "\n" + line;
		}
		ifstream.close();
	}
	else {
		throw IOException("could not open file: " + std::string(shaderfile), LOGGER_DETAIL_DEFAULT);
	}
	return rgle::Shader::compile(file, type);
}

GLuint rgle::Shader::compile(std::string shadercode, GLenum type)
{
	RGLE_DEBUG_ONLY(rgle::Logger::debug("compiling shader of type: " + std::to_string(type), LOGGER_DETAIL_DEFAULT);)
	GLuint shaderId = glCreateShader(type);
	GLint result = GL_FALSE;
	int logLength;
	char const* ptr = shadercode.c_str();
	glShaderSource(shaderId, 1, &ptr, nullptr);
	glCompileShader(shaderId);

	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetShaderInfoLog(shaderId, logLength, nullptr, &errorMessage[0]);
		throw Exception(std::string("failed to compile shader: ") + errorMessage.data(), LOGGER_DETAIL_DEFAULT);
	}
	return shaderId;
}

rgle::ShaderProgram::ShaderProgram()
{
}

rgle::ShaderProgram::ShaderProgram(std::string name, std::string vertexShader, std::string fragmentShader) : ShaderProgram(
	name,
	vertexShader.c_str(),
	fragmentShader.c_str()
) {}

rgle::ShaderProgram::ShaderProgram(std::string name, const char * vertexShader, const char * fragmentShader) :
	ShaderProgram(
		name,
		{
			Shader::compileFile(vertexShader, GL_VERTEX_SHADER),
			Shader::compileFile(fragmentShader, GL_FRAGMENT_SHADER)
		}
	)
{}

rgle::ShaderProgram::ShaderProgram(std::string name, std::initializer_list<GLuint> shaders)
{
	Logger::info("creating shader program", LOGGER_DETAIL_IDENTIFIER(name));
	this->id = name;
	this->_programID = glCreateProgram();
	for (GLuint id : shaders) {
		glAttachShader(this->_programID, id);
	}
	glLinkProgram(this->_programID);

	GLint result = GL_FALSE;
	int logLength;

	glGetProgramiv(this->_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(this->_programID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		std::vector<char> errorMessage(logLength + 1);
		glGetProgramInfoLog(this->_programID, logLength, nullptr, &errorMessage[0]);
		throw Exception(std::string("failed to link shader program: ") + errorMessage.data(), LOGGER_DETAIL_IDENTIFIER(id));
	}

	for (GLuint id : shaders) {
		glDetachShader(this->_programID, id);
		glDeleteShader(id);
	}
}


rgle::ShaderProgram::~ShaderProgram()
{
}

GLuint rgle::ShaderProgram::programId() const
{
	return _programID;
}

void rgle::ShaderProgram::use() const
{
	glUseProgram(_programID);
}

const char * rgle::ShaderProgram::typeName() const
{
	return "rgle::ShaderProgram";
}

GLint rgle::ShaderProgram::uniform(const std::string & name) const
{
	return glGetUniformLocation(this->programId(), name.c_str());
}

GLint rgle::ShaderProgram::uniformStrict(const std::string & name) const
{
	GLint result = this->uniform(name);
	if (result < 0) {
		throw NotFoundException("failed to locate uniform '" + name + "' in program '" + this->id + '\'', LOGGER_DETAIL_IDENTIFIER(this->id));
	}
	else {
		return result;
	}
}

rgle::ShaderManager::ShaderManager()
{

}

rgle::ShaderManager::ShaderManager(std::initializer_list<std::shared_ptr<ShaderProgram>> programs)
{
	this->_shaderPrograms = std::vector<std::shared_ptr<ShaderProgram>>();
	for (const auto& program : programs) {
		this->_shaderPrograms.push_back(program);
	}
}

rgle::ShaderManager::~ShaderManager()
{
}

void rgle::ShaderManager::addShader(std::shared_ptr<ShaderProgram> shader)
{
	for (int i = 0; i < this->_shaderPrograms.size(); i++) {
		if (shader->id == this->_shaderPrograms[i]->id) {
			throw IdentifierException("shader program already exists", shader->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	this->_shaderPrograms.push_back(shader);
}

std::shared_ptr<rgle::ShaderProgram> rgle::ShaderManager::operator[](std::string name)
{
	for (int i = 0; i < this->_shaderPrograms.size(); i++) {
		if (this->_shaderPrograms[i]->id == name) {
			return this->_shaderPrograms[i];
		}
	}
	return nullptr;
}

std::shared_ptr<rgle::ShaderProgram> rgle::ShaderManager::get(std::string name) const
{
	for (int i = 0; i < this->_shaderPrograms.size(); i++) {
		if (this->_shaderPrograms[i]->id == name) {
			return this->_shaderPrograms[i];
		}
	}
	return nullptr;
}

std::shared_ptr<rgle::ShaderProgram> rgle::ShaderManager::getStrict(std::string name) const
{
	auto result = this->get(name);
	if (result == nullptr) {
		throw NotFoundException("failed to lookup shader '" + name + '\'', LOGGER_DETAIL_DEFAULT);
	}
	return result;
}

void APIENTRY rgle::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam)
{
	std::string msg = message;
	msg = msg + " (type: " + std::to_string(type) + ", severity: " + std::to_string(severity);
	if (userParam != nullptr) {
		msg = msg + ", params: " + (const char*)userParam;
	}
	msg = msg + ')';
	if (type == GL_DEBUG_TYPE_ERROR) {
		rgle::Logger::error(msg, LOGGER_DETAIL_DEFAULT);
	}
	else {
		// TODO: implement GL debug log
		//RGLE_DEBUG_ONLY(rgle::Logger::debug(msg, LOGGER_DETAIL_DEFAULT);)
	}
}
