#pragma once

#include "Platform.h"

#include <iostream>
#include <cstdio>
#include <string>

namespace rgle {
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
