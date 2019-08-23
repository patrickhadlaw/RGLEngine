#include "Logger.h"
#include "Exception.h"


std::string rgle::Logger::_logFile = Logger::_getDefaultLogFilename();
std::ofstream rgle::Logger::_outputStream = std::ofstream();

std::string rgle::Logger::timeString(std::chrono::time_point<std::chrono::system_clock>& timestamp)
{
	auto itt = std::chrono::system_clock::to_time_t(timestamp);
	std::stringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

void rgle::Logger::message(std::string message)
{
	std::cout << message << std::endl;
	Logger::write(message);
}

void rgle::Logger::info(std::string message, Detail & detail)
{
	if (Settings::getLogLevel() <= LogLevel::INFO) {
		Logger::write(Logger::_print("INFO", message, detail, Console::Color::CYAN));
	}
}

void rgle::Logger::debug(std::string message, Detail & detail)
{
	if (Settings::getLogLevel() <= LogLevel::DEBUG) {
		Logger::write(Logger::_print("DEBUG", message, detail));
	}
}

void rgle::Logger::warn(std::string warning, Detail & detail)
{
	if (Settings::getLogLevel() <= LogLevel::WARN) {
		Logger::write(Logger::_print("WARN", warning, detail, Console::Color::YELLOW));
	}
}

void rgle::Logger::error(std::string error, Detail & detail)
{
	if (Settings::getLogLevel() <= LogLevel::ERROR) {
		Logger::write(Logger::_print("ERROR", error, detail, Console::Color::RED));
	}
}

void rgle::Logger::except(Exception* except)
{
	if (except != nullptr) {
		Logger::write(except->print());
	}
	else {
		Logger::warn("Attempt to log null exception", LOGGER_DETAIL_DEFAULT);
	}
}

void rgle::Logger::write(std::string entry)
{
	if (Settings::getLoggerWrite()) {
		if (!Logger::_outputStream.is_open()) {
			std::filesystem::create_directory("logs");
			Logger::_outputStream.open(Logger::_logFile);
			if (!Logger::_outputStream) {
				Settings::setLoggerWrite(false);
				throw IOException(std::string("failed to open log file: ") + Logger::_logFile, LOGGER_DETAIL_DEFAULT);
			}
		}
		Logger::_outputStream << entry << std::endl;
	}
}

std::string rgle::Logger::_print(std::string header, std::string& message, Detail& detail, Console::Color color)
{
	std::string time = Logger::timeString(detail.timestamp);
	std::cout << time << '|';
	Console::coloredPrint(color, header);
	std::string result = std::string("|") + detail.file + '|' + detail.func + '|' + std::to_string(detail.line);
	result += detail.id.empty() ? "" : std::string("|ID: ") + detail.id + "|";
	result += std::string(": ") + message;
	std::cout << result << std::endl;
	result = time + '|' + header + result;
	return result;
}

std::string rgle::Logger::_getDefaultLogFilename()
{
	std::string file = rgle::Logger::timeString(std::chrono::system_clock::now());
	std::replace(file.begin(), file.end(), ':', '-');
	return "logs/" + file + ".log";
}
