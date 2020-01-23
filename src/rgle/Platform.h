#pragma once

#include "configuration.h"

#include <iostream>
#include <atomic>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <ctime>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <array>
#include <cctype>
#include <memory>
#include <map>
#include <assert.h>
#include <thread>
#include <mutex>

#if defined _MSC_VER

#ifdef RGLE_DLL_BUILD_MODE
#define RGLE_DLLEXPORTED  __declspec(dllexport)
#else
#define RGLE_DLLEXPORTED  __declspec(dllimport)
#endif

#else

#define RGLE_DLLEXPORTED

#endif

namespace rgle {

	enum class LogLevel {
		DEBUG,
		INFO,
		WARN,
		ERROR,
		NONE
	};

	LogLevel get_loglevel_from_string(std::string loglevel);

	std::string installed_filename(std::string filename);

	namespace Platform {
		void initialize();
	}

	class Config;

	class Settings {
		friend void Platform::initialize();
	public:
		static std::string get(std::string key);

		static bool getLoggerWrite();
		static void setLoggerWrite(bool write);

		static LogLevel getLogLevel();
		static void setLogLevel(LogLevel level);
		
		static RGLE_DLLEXPORTED const std::string INSTALL_PATH;
		static RGLE_DLLEXPORTED const std::string LOG_LEVEL;
	private:
		static RGLE_DLLEXPORTED std::atomic_bool _loggerWrite;
		static RGLE_DLLEXPORTED std::atomic<LogLevel> _logLevel;
		static RGLE_DLLEXPORTED Config _config;
	};
}

#ifdef NDEBUG
#define RGLE_DEBUG_ONLY(x)
#else
#define RGLE_DEBUG_ONLY(x) x
#endif

#ifdef TARGET_OS_MAC
#define RGLE_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined __linux__
#define RGLE_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined _WIN32 || defined _WIN64
#define RGLE_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#error "Unknown platform..."
#endif