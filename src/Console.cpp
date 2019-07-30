#include "Console.h"

#if defined _WIN32 || defined _WIN64
#include <windows.h>
void cppogl::Console::coloredPrint(Color color, std::string text)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO cbInfo;
	GetConsoleScreenBufferInfo(hConsole, &cbInfo);
	WORD defaultBackground = cbInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	switch (color) {
	case cppogl::Console::Color::RED:
		SetConsoleTextAttribute(hConsole,
			FOREGROUND_RED | FOREGROUND_INTENSITY | defaultBackground);
		break;
	case cppogl::Console::Color::GREEN:
		SetConsoleTextAttribute(hConsole,
			FOREGROUND_GREEN | FOREGROUND_INTENSITY | defaultBackground);
		break;
	}
	std::cout << text;
	SetConsoleTextAttribute(hConsole, cbInfo.wAttributes);
}
#else
void cppogl::Console::coloredPrint(Color color, std::string text)
{
	switch (color) {
	case cppogl::Console::Color::RED:
		std::cout << "\033[1;31m" << text;
		break;
	case cppogl::Console::Color::GREEN:
		std::cout << "\033[1;32m" << text;
		break;
	}
	std::cout << "\033[0;";
}
#endif

