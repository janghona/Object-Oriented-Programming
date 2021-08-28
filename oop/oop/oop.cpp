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
			instance = new Screen(10,10);
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

int main()
{
	Screen* screen = Screen::getInstance();

	bool requestExit = false;
	int x = 0, y = 0;
	
	while (requestExit == false) {
		screen->clear();
		
		screen->draw(x, y, '0' + x);
		screen->render();

		//debugging
		Borland::gotoxy(20, 20);
		printf("x = %d, y = %d", x, y);

		Sleep(100);
	}
	printf("\nGame Over\n");
	return 0;
}