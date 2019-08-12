#include "Platform.h"


void rgle::Platform::initialize() {

}

std::atomic_bool rgle::Settings::_loggerWrite = true;

#ifdef NDEBUG
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::INFO;
#else
std::atomic<rgle::LogLevel> rgle::Settings::_logLevel = rgle::LogLevel::DEBUG;
#endif

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