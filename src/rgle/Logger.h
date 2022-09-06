#pragma once

#include "rgle/util/Console.h"

#define LOGGER_DETAIL_DEFAULT rgle::Logger::Detail{std::chrono::system_clock::now(), RGLE_FILENAME, __func__, __LINE__}
#define LOGGER_DETAIL_IDENTIFIER(id) rgle::Logger::Detail{std::chrono::system_clock::now(), RGLE_FILENAME, __func__, __LINE__, id}

namespace rgle {

	class Exception;

	class Logger {
	public:
		struct Detail {
			std::chrono::time_point<std::chrono::system_clock> timestamp;
			std::string file = "";
			std::string func = "";
			int line = 0;
			std::string id = "";
		};

		static std::string timeString(std::chrono::time_point<std::chrono::system_clock> timestamp);
		static std::string detailString(Detail& detail);

		static void message(std::string message);
		static void info(std::string message, Detail detail);
		static void debug(std::string message, Detail detail);
		static void warn(std::string warning, Detail detail);
		static void error(std::string error, Detail detail);
		static void except(Exception* except);

		static void write(std::string entry);
		
	private:

		static RGLE_DLLEXPORTED std::string _logFile;
		static RGLE_DLLEXPORTED std::ofstream _outputStream;

		static std::string _print(
			std::string header,
			std::string& message,
			Detail& detail,
			util::Console::Color color = util::Console::Color::DEFAULT
		);

		static std::string _getDefaultLogFilename();
	};
}
