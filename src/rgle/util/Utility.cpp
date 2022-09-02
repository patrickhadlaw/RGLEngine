#include "rgle/util/Utility.h"


rgle::DebugException::DebugException(std::string except, Logger::Detail detail) : Exception(except, detail, "rgle::DebugException")
{
}

void rgle::debug_assert(bool assert, Logger::Detail detail)
{
	if (!assert) {
		throw DebugException("assertion failed", detail);
	}
}

unsigned char rgle::random_byte()
{
	std::random_device rd;
	std::default_random_engine random;
	random.seed(rd());
	std::uniform_int_distribution<int> distribution(0, 255);
	return static_cast<unsigned char>(distribution(random));
}

std::string rgle::uid()
{
	std::stringstream ss;
	for (int i = 0; i < UID_LENGTH; i++) {
		const unsigned char byte = random_byte();
		std::stringstream hex;
		hex << std::hex << std::setfill('0') << std::setw(2) << (int)byte;
		ss << hex.str();
	}
	return ss.str();
}
