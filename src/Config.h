#pragma once

#include "Exception.h"

namespace rgle {
	class ConfigException : public Exception {
	public:
		ConfigException(std::string exception, Logger::Detail& detail);
		virtual ~ConfigException();

	protected:
		virtual std::string _type();
	};

	class Config {
	public:
		Config();
		Config(std::string filename);

		std::string operator[](std::string entry) const;

	private:
		enum class ParseState {
			KEY,
			EQUAL,
			VALUE
		};

		bool _validate(char c);

		std::map<std::string, std::string> _config;
	};
}