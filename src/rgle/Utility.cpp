#include "rgle/Utility.h"


rgle::DebugException::DebugException(std::string except, Logger::Detail& detail) : Exception(except, detail, "rgle::DebugException")
{
}

void rgle::debug_assert(bool assert, Logger::Detail & detail)
{
	if (!assert) {
		throw DebugException("assertion failed", detail);
	}
}
