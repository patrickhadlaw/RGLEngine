#pragma once

#include "rgle/Exception.h"

#define RGLE_DEBUG_ASSERT(x) RGLE_DEBUG_ONLY(rgle::debug_assert(x, LOGGER_DETAIL_DEFAULT);)

namespace rgle {
	class DebugException : public Exception {
	public:
		DebugException(std::string except, Logger::Detail& detail);
	};

	void debug_assert(bool assert, Logger::Detail& detail);
}