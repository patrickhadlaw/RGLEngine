#pragma once

#include "configuration.h"

// Suppress warnings from external libraries
#if defined _MSC_VER
	#pragma warning(push, 0)
	#pragma warning(push)
	#pragma warning(disable:4244)
#elif defined __GNUG__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wuninitialized"
#else
	#warning "Unidentified compiler..."
#endif

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <array>
#include <cctype>
#include <assert.h>
#include <deque>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <optional>
#include <variant>

#include <GL\glew.h>
#include <GL\GL.h>
#include <GLFW\glfw3.h>
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_SWIZZLE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <ft2build.h>
#include <freetype\ftglyph.h>
#include FT_FREETYPE_H

#if defined _MSC_VER
	#pragma warning(pop)
	#pragma warning(pop)
#elif defined __GNUG__
	#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
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

	std::string get_executable_path();

	LogLevel get_loglevel_from_string(std::string loglevel);

	namespace Platform {
		void initialize();
	}

	class Config;
	class Tester;

	class Settings {
		friend void Platform::initialize();
		friend class Tester;
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
		static RGLE_DLLEXPORTED std::atomic_bool _initialized;
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
