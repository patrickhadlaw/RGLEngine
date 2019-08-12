#pragma once

#include "configuration.h"

#include <iostream>
#include <string.h>
#include <atomic>

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

	namespace Platform {
		void initialize();
	}
	class Settings {
	public:
		static bool getLoggerWrite();
		static void setLoggerWrite(bool write);

		static LogLevel getLogLevel();
		static void setLogLevel(LogLevel level);
	private:
		static RGLE_DLLEXPORTED std::atomic_bool _loggerWrite;
		static RGLE_DLLEXPORTED std::atomic<LogLevel> _logLevel;
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
