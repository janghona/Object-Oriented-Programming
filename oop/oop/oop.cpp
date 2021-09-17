#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string> // c++ string class
#include <Windows.h>
#include"Utils.h"
using namespace std;

class Screen {
private:
	int width;
	int height;
	char* buffer;

	static Screen* instance;

	Screen(int width = 10, int height = 10)
		: width(width), height(height), buffer(new char[getSize()]) {
		Borland::initialize();
		buffer[getSize() - 1] = '\0';
	}
public:

	static Screen* getInstance() {
		if (instance == nullptr) {
			instance = new Screen(80,20);
		}
		return instance;
	}
	~Screen() { delete[] buffer; }

	int getWidth() const { return width; }
	int getScreenWidth() const { return width + 1; }
	int getHeight() const { return height; }
	int getSize() const { return getScreenWidth() * height; }

	void clear() {
		memset(buffer, '*', getSize());
		buffer[getSize() - 1] = '\0';
	}

	void render() {
		for (int y = 0; y < height - 1; y++) {
			buffer[y * getScreenWidth() + width] = '\n';
		}
		buffer[getSize() - 1] = '\0';
		Borland::gotoxy(0, 0);
		cout << buffer;
	}

	void draw(int x, int y, char shape) {
		buffer[y * getScreenWidth() + x] = shape;
	}
};

Screen* Screen::instance = nullptr;

HANDLE hStdin;
DWORD fdwSaveOldMode;

VOID ErrorExit(const char *);
VOID KeyEventProc(KEY_EVENT_RECORD);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

int main()
{
	Screen* screen = Screen::getInstance();

	DWORD cNumRead, fdwMode;
	INPUT_RECORD irInBuf[128];

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
		ErrorExit("GetStdHandle");

	if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
		ErrorExit("GetConsoleMode");

	// Enable the window and mouse input events. 

	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(hStdin, fdwMode))
		ErrorExit("SetConsoleMode");

	bool requestExit = false;
	int x = 0, y = 0;
	
	while (requestExit == false) {
		screen->clear();
		
		if (!ReadConsoleInput(
			hStdin,      // input buffer handle 
			irInBuf,     // buffer to read into 
			128,         // size of read buffer 
			&cNumRead)) // number of records read 
			ErrorExit("ReadConsoleInput");

		// Dispatch the events to the appropriate handler. 
		for (int i = 0; i < cNumRead; i++)
		{
			switch (irInBuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input 
				KeyEventProc(irInBuf[i].Event.KeyEvent);
				break;

			case MOUSE_EVENT: // mouse input 
				MouseEventProc(irInBuf[i].Event.MouseEvent);
				break;

			case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
				ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent);
				break;

			case FOCUS_EVENT:  // disregard focus events 

			case MENU_EVENT:   // disregard menu events 
				break;

			default:
				ErrorExit("Unknown event type");
				break;
			}
		}

		screen->draw(x, y, '0' + x);
		screen->render();

		//debugging
		Borland::gotoxy(0, 21);
		printf("cNumRead = %d", cNumRead);

		Sleep(100);
	}
	printf("\nGame Over\n");
	return 0;
}

VOID ErrorExit(const char* lpszMessage){
	Borland::gotoxy(0, 21);
	printf("%80d\r", ' ');
	printf("%s\n", lpszMessage);

	// Restore input mode on exit.
	SetConsoleMode(hStdin, fdwSaveOldMode);
	ExitProcess(0);
}

VOID KeyEventProc(KEY_EVENT_RECORD ker){
	Borland::gotoxy(0, 22);
	printf("%80d\r", ' ');

	printf("Key event: %c  %d        ", ker.uChar, ker.wRepeatCount);

	if (ker.bKeyDown)
		printf("key pressed\n");
	else printf("key released\n");
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer){
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
	Borland::gotoxy(0, 22);
	printf("%80d\r", ' ');

	printf("Mouse event: %d %d      ", mer.dwMousePosition.X, mer.dwMousePosition.Y);

	switch (mer.dwEventFlags){
	case 0:
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED){
			printf("left button press \n");
		}
		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED){
			printf("right button press \n");
		}
		else{
			printf("button press\n");
		}
		break;
	case DOUBLE_CLICK:
		printf("double click\n");
		break;
	case MOUSE_HWHEELED:
		printf("horizontal mouse wheel\n");
		break;
	case MOUSE_MOVED:
		printf("mouse moved\n");
		break;
	case MOUSE_WHEELED:
		printf("vertical mouse wheel\n");
		break;
	default:
		printf("unknown\n");
		break;
	}
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr){
	Borland::gotoxy(0, 22);
	printf("%80d\r", ' ');
	printf("Resize event:             ");
	printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}