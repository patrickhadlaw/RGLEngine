#pragma once

#include "rgle/Exception.h"

namespace rgle::util {
	class ConfigException : public Exception {
	public:
		ConfigException(std::string exception, Logger::Detail detail);
	};

	class Config {
	public:
		enum class Type {
			OBJECT,
			LIST,
			STRING,
			NUMBER
		};
		Config();
		Config(std::string filename);

		int getInt(std::string entry);
		float getFloat(std::string entry);

		std::string operator[](std::string entry) const;

	private:
		enum class ParseState {
			KEY,
			EQUAL,
			VALUE
		};

		bool _validate(char c);

		size_t _iterateWhitespace(std::string line, size_t index);

		std::map<std::string, std::string> _config;
	};
}