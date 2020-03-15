#include "rgle/Platform.h"
#include "rgle/Config.h"


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

std::string rgle::installed_filename(std::string filename) {
	std::stringstream ss;
	ss << Settings::get(Settings::INSTALL_PATH) << '/' << filename;
	return ss.str();
}

void rgle::Platform::initialize() {
	Settings::_config = Config("rgle.cfg");
	Settings::_logLevel = rgle::get_loglevel_from_string(Settings::get(Settings::LOG_LEVEL));
	if (Settings::_logLevel == LogLevel::NONE) {
		Settings::_loggerWrite = false;
	}
}

const std::string rgle::Settings::INSTALL_PATH = "install_path";
const std::string rgle::Settings::LOG_LEVEL = "log_level";

std::atomic_bool rgle::Settings::_loggerWrite = true;
rgle::Config rgle::Settings::_config = Config();

#ifdef NDEBUG
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::INFO;
#else
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::DEBUG;
#endif

std::string rgle::Settings::get(std::string key) {
	return Settings::_config[key];
}

bool rgle::Settings::getLoggerWrite() {
	return Settings::_loggerWrite.load();
}

void rgle::Settings::setLoggerWrite(bool write) {
	Settings::_loggerWrite.store(write);
}

rgle::LogLevel rgle::Settings::getLogLevel() {
	return Settings::_logLevel.load();
}

void rgle::Settings::setLogLevel(LogLevel level) {
	Settings::_logLevel.store(level);
}