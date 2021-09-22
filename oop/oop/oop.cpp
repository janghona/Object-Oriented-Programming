#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string> // c++ string class
#include <Windows.h>
#include<deque>
#include<vector>
#include"Utils.h"
using namespace std;

class Screen {
private:
	int width;
	int height;
	char* buffer;

	Screen(int width = 10, int height = 10)
		: width(width), height(height), buffer(new char[getSize()]) {
		Borland::initialize();
		buffer[getSize() - 1] = '\0';
	}
	static Screen* instance;
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
	void draw(int x, int y, const char* shape) {
		strncpy(&buffer[y*getScreenWidth() + x], shape, strlen(shape));
	}
};
Screen* Screen::instance = nullptr;

class InputManager {
private:
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	DWORD fdwMode;
	INPUT_RECORD irInBuf[128];
	deque<INPUT_RECORD> events;

	VOID ErrorExit(const char* lpszMessage) {
		// Restore input mode on exit.
		SetConsoleMode(hStdin, fdwSaveOldMode);
		ExitProcess(0);
	}

	VOID KeyEventProc(KEY_EVENT_RECORD ker) {
		if (ker.bKeyDown)
			printf("key pressed\n");
		else printf("key released\n");
	}

	VOID MouseEventProc(MOUSE_EVENT_RECORD mer) {
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
		Borland::gotoxy(0, 22);
		printf("%80d\r", ' ');
		printf("Mouse event: %d %d      ", mer.dwMousePosition.X, mer.dwMousePosition.Y);

		switch (mer.dwEventFlags) {
		case 0:
			if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
				printf("left button press \n");
			}
			else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
				printf("right button press \n");
			}
			else {
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

	InputManager() :hStdin(GetStdHandle(STD_INPUT_HANDLE)), irInBuf{ {0} }
	{
		if (hStdin == INVALID_HANDLE_VALUE) ErrorExit("GetStdHandle");
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) ErrorExit("GetConsoleMode");

		// Enable the window and mouse input events. 
		fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		if (!SetConsoleMode(hStdin, fdwMode)) ErrorExit("SetConsoleMode");
		events.clear();
	}
	static InputManager* instance;
public:
	static InputManager* getInstance() {
		if (instance == nullptr) {
			instance = new InputManager();
		}
		return instance;
	}

	bool GetKeyDown(WORD ch) {
		if (events.empty() == true) return false;
		INPUT_RECORD& in = events.front();
		if (in.EventType != KEY_EVENT) return false;
		if (in.Event.KeyEvent.bKeyDown = TRUE) {
			return in.Event.KeyEvent.wVirtualKeyCode == ch;
		}
		return false;
	}

	void readInputs() {
		DWORD cNumRead;
		DWORD nEvents;

		if (!GetNumberOfConsoleInputEvents(hStdin, &nEvents)) return;
		if (nEvents == 0) return;
		nEvents = min(nEvents, 128);

		ReadConsoleInput(
			hStdin,      // input buffer handle 
			irInBuf,     // buffer to read into 
			nEvents,         // size of read buffer 
			&cNumRead); // number of records read 
			
		for (int i = 0; i < cNumRead; i++){
			events.push_back(irInBuf[i]);
		}
	}

	void consumeEvent() {
		if (events.empty() == true) return;
		events.pop_front();
	}
};
InputManager* InputManager::instance = nullptr;

class GameObject {
private:
	string shape;
	Position pos;
protected:
	Screen& screen;
	InputManager& inputManager;
public:
	GameObject(int x,int y,const string& shape)
		:shape(shape),pos(x,y),screen(*Screen::getInstance()),inputManager(*InputManager::getInstance())
	{

	}
	virtual ~GameObject()
	{
	}

	void setPos(int x, int y) { pos.x = x; pos.y = y; }
	void setPos(const Position& pos) { this->pos.x = pos.x; this->pos.y = pos.y; }
	Position getPos() const { return pos; }

	virtual void update() {}
	virtual void draw() { screen.draw(pos.x, pos.y, shape.c_str()); }
};

class Block : public GameObject {

public:
	Block(const string& shape = "(^_^)",int x = 5, int y = 5)
		: GameObject(x, y, shape) {}

	void update() override{
		Position pos = getPos();
		if (inputManager.GetKeyDown(VK_LEFT) == true) {
			setPos(pos + Position::left);
		}
		if (inputManager.GetKeyDown(VK_RIGHT) == true) {
			setPos(pos + Position::right);
		}
		if (inputManager.GetKeyDown(VK_UP) == true) {
			setPos(pos + Position::up);
		}
		if (inputManager.GetKeyDown(VK_DOWN) == true) {
			setPos(pos + Position::down);
		}
	}
};

int main()
{
	Screen& screen = *Screen::getInstance();
	InputManager& inputmanager = *InputManager::getInstance();
	//Block block;
	vector<GameObject*> gameObjects;
	gameObjects.push_back(new Block{ "(^_^)", 5, 5 });
	gameObjects.push_back(new Block{ "(-_-)", 10, 10 });

	bool requestExit = false;
	int x = 0, y = 0;
	
	while (requestExit == false) {
		screen.clear();
	
		inputmanager.readInputs();

		for (auto object : gameObjects) object->update();
		for (auto object : gameObjects) object->draw();
		//block.update();
		//block.draw();

		screen.render();

		inputmanager.consumeEvent();

		Sleep(100);
	}
	printf("\nGame Over\n");
	return 0;
}
