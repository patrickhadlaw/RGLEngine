#include "rgle/util/Console.h"

#if defined _WIN32 || defined _WIN64
	#pragma warning(push, 0)
	#include <windows.h>
	void rgle::Console::coloredPrint(Color color, std::string text)
	{
		if (color != rgle::Console::Color::DEFAULT) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO cbInfo;
			GetConsoleScreenBufferInfo(hConsole, &cbInfo);
			WORD defaultBackground = cbInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
			switch (color) {
			case rgle::Console::Color::RED:
				SetConsoleTextAttribute(hConsole,
					0x4 | FOREGROUND_INTENSITY | defaultBackground);
				break;
			case rgle::Console::Color::GREEN:
				SetConsoleTextAttribute(hConsole,
					0x2 | FOREGROUND_INTENSITY | defaultBackground);
				break;
			case rgle::Console::Color::YELLOW:
				SetConsoleTextAttribute(hConsole,
					0x6 | FOREGROUND_INTENSITY | defaultBackground);
				break;
			case rgle::Console::Color::CYAN:
				SetConsoleTextAttribute(hConsole,
					0x3 | FOREGROUND_INTENSITY | defaultBackground);
				break;
			}
			std::cout << text;
			SetConsoleTextAttribute(hConsole, cbInfo.wAttributes);
		}
		else {
			std::cout << text;
		}
	}
	#pragma warning(pop)
#else
	void rgle::Console::coloredPrint(Color color, std::string text)
	{
		if (color != rgle::Console::Color::DEFAULT) {
			switch (color) {
			case rgle::Console::Color::RED:
				std::cout << "\033[1;31m" << text;
				break;
			case rgle::Console::Color::GREEN:
				std::cout << "\033[1;32m" << text;
				break;
			case rgle::Console::Color::YELLOW:
				std::cout << "\033[1;33m" << text;
				break;
			case rgle::Console::Color::CYAN:
				std::cout << "\033[1;36m" << text;
				break;
			}
			std::cout << "\033[0;";
		}
		else {
			std::cout << text;
		}
	}
#endif

