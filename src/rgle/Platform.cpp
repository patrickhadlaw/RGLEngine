#include "rgle/Platform.h"
#include "rgle/util/Config.h"


#if defined _WIN32 || defined _WIN64
	#pragma warning(push, 0)        
	#include <windows.h>
	std::string rgle::get_executable_path()
	{
		char buf[4096];
		if (GetModuleFileNameA(NULL, buf, 4096) == 0) {
			throw std::runtime_error("failed to read executable location...");
		}
		return std::string(buf);
	}
	#pragma warning(pop)        

	// Workaround for windows.h defining ERROR which conflicts with LogLevel::ERROR
	#undef ERROR
#else
	#include <unistd.h>
	std::string rgle::get_executable_path()
	{
		char buf[4096];
		ssize_t result = readlink("/proc/self/exe", buf, 4096);
		if (result < 0) {
			throw std::runtime_error("failed to read executable location...");
		}
		return std::string(buf);
	}
#endif

rgle::LogLevel rgle::get_loglevel_from_string(std::string loglevel)
{
	if (loglevel == "DEBUG") {
		return LogLevel::DEBUG;
	}
	else if (loglevel == "INFO") {
		return LogLevel::INFO;
	}
	else if (loglevel == "WARN") {
		return LogLevel::WARN;
	}
	else if (loglevel == "ERROR") {
		return LogLevel::ERROR;
	}
	else {
		return LogLevel::NONE;
	}
}

void rgle::Platform::initialize() {
	if (!Settings::_initialized) {
		Settings::_config = Config(std::filesystem::path(rgle::get_executable_path()).remove_filename().string() + "rgle.cfg");
		Settings::_initialized = true;
		std::filesystem::current_path(Settings::get(Settings::INSTALL_PATH));
		Settings::_logLevel = rgle::get_loglevel_from_string(Settings::get(Settings::LOG_LEVEL));
		if (Settings::_logLevel == LogLevel::NONE) {
			Settings::_loggerWrite = false;
		}
	}
}

const std::string rgle::Settings::INSTALL_PATH = "install_path";
const std::string rgle::Settings::LOG_LEVEL = "log_level";

std::atomic_bool rgle::Settings::_loggerWrite = true;
rgle::Config rgle::Settings::_config = Config();
std::atomic_bool rgle::Settings::_initialized = false;

#ifdef NDEBUG
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::INFO;
#else
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::DEBUG;
#endif

std::string rgle::Settings::get(std::string key) {
	if (!Settings::_initialized) {
		throw std::runtime_error("failed to get settings, platform uninitialized");
	}
	return Settings::_config[key];
}

bool rgle::Settings::getLoggerWrite() {
	if (!Settings::_initialized) {
		throw std::runtime_error("failed to get settings, platform uninitialized");
	}
	return Settings::_loggerWrite.load();
}

void rgle::Settings::setLoggerWrite(bool write) {
	if (!Settings::_initialized) {
		throw std::runtime_error("failed to get settings, platform uninitialized");
	}
	Settings::_loggerWrite.store(write);
}

rgle::LogLevel rgle::Settings::getLogLevel() {
	if (!Settings::_initialized) {
		throw std::runtime_error("failed to get settings, platform uninitialized");
	}
	return Settings::_logLevel.load();
}

void rgle::Settings::setLogLevel(LogLevel level) {
	if (!Settings::_initialized) {
		throw std::runtime_error("failed to get settings, platform uninitialized");
	}
	Settings::_logLevel.store(level);
}