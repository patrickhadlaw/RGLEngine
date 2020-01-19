#include "rgle/Config.h"

rgle::Config::Config()
{
}

rgle::Config::Config(std::string filename)
{
	std::ifstream config(filename);
	if (config.is_open()) {
		for (std::string line; std::getline(config, line);) {
			if (!line.empty()) {
				std::string key, value;
				bool quote = false;
				size_t valueindex;
				ParseState state = ParseState::KEY;
				for (int i = 0; i < line.length(); i++) {
					if (state == ParseState::KEY) {
						if (line[i] == '=') {
							key = line.substr(0, i);
							value = line.substr(i + 1, std::string::npos);
							state = ParseState::EQUAL;
						}
						else if (!this->_validate(line[i])) {
							throw ConfigException("failed to load config: " + filename + ", invalid format", LOGGER_DETAIL_DEFAULT);
						}
					}
					else if (state == ParseState::EQUAL) {
						if (line[i] == '\"') {
							quote = true;
							valueindex = i + 1;
						}
						else if (!this->_validate(line[i])) {
							throw ConfigException("failed to load config: " + filename + ", invalid format", LOGGER_DETAIL_DEFAULT);
						}
						else {
							valueindex = i;
						}
						state = ParseState::VALUE;
					}
					else if (state == ParseState::VALUE) {
						if (line[i] == '\"') {
							if (i == line.length() - 1) {
								value = line.substr(valueindex, line.length() - valueindex - 1);
								quote = false;
							}
							else {
								throw ConfigException("failed to load config: " + filename + ", invalid format", LOGGER_DETAIL_DEFAULT);
							}
						}
						else if (!quote && !this->_validate(line[i])) {
							throw ConfigException("failed to load config: " + filename + ", invalid format", LOGGER_DETAIL_DEFAULT);
						}
					}
					
				}
				if (quote) {
					throw ConfigException("failed to load config: " + filename + ", missing end quote", LOGGER_DETAIL_DEFAULT);
				}
				this->_config[key] = value;
			}
		}
		config.close();
	}
	else {
		throw IOException("failed to open file: " + filename, LOGGER_DETAIL_DEFAULT);
	}
}

int rgle::Config::getInt(std::string entry)
{
	return 0;
}

float rgle::Config::getFloat(std::string entry)
{
	return 0.0f;
}

std::string rgle::Config::operator[](std::string entry) const
{
	if (this->_config.find(entry) == this->_config.end()) {
		throw ConfigException("failed to lookup config entry: " + entry + ", entry not found", LOGGER_DETAIL_DEFAULT);
	}
	else {
		return this->_config.at(entry);
	}
}

bool rgle::Config::_validate(char c)
{
	return c == '_' || c == '-' || std::isalnum(static_cast<unsigned char>(c));
}

rgle::ConfigException::ConfigException(std::string exception, Logger::Detail & detail) : Exception(exception, detail)
{
}

rgle::ConfigException::~ConfigException()
{
}

std::string rgle::ConfigException::_type()
{
	return std::string("rgle::ConfigException");
}
