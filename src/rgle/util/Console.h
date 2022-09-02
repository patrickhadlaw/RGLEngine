#pragma once

#include "rgle/Platform.h"

namespace rgle::util {
	namespace Console {
		enum class Color {
			DEFAULT,
			RED,
			GREEN,
			YELLOW,
			CYAN
		};

		void coloredPrint(Color color, std::string text);
	};
}
