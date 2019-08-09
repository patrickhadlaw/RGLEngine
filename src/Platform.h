#pragma once

#include <iostream>
#include <string.h>

namespace cppogl {
	namespace Platform {
		void initialize();
	}
}

#if defined _MSC_VER

#ifdef CPPOGL_DLL_BUILD_MODE
#define CPPOGL_DLLEXPORTED  __declspec(dllexport)
#else
#define CPPOGL_DLLEXPORTED  __declspec(dllimport)
#endif

#else

#define CPPOGL_DLLEXPORTED

#endif

#ifdef TARGET_OS_MAC
#define CPPOGL_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined __linux__
#define CPPOGL_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined _WIN32 || defined _WIN64
#define CPPOGL_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#error "Unknown platform..."
#endif
