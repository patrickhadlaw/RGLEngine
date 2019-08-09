#pragma once

#include <iostream>
#include <string.h>

namespace rgle {
	namespace Platform {
		void initialize();
	}
}

#if defined _MSC_VER

#ifdef RGLE_DLL_BUILD_MODE
#define RGLE_DLLEXPORTED  __declspec(dllexport)
#else
#define RGLE_DLLEXPORTED  __declspec(dllimport)
#endif

#else

#define RGLE_DLLEXPORTED

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
