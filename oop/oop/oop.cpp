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
			draw(width, y, '\n');
		}
		draw(width-1,height,'\0');
		Borland::gotoxy(0, 0);
		cout << buffer;
	}

	void draw(int x, int y, const char shape) { buffer[y* getScreenWidth() + x] = shape; }
	void draw(int x, int y, const char* shape) {
		if (shape == nullptr) return;
		strncpy(&buffer[y*getScreenWidth() + x], shape, strlen(shape));
	}
	void draw(const Position& pos, char shape) { draw(pos.x, pos.y, shape); }
	void draw(const Position& pos, const char* shape) { draw(pos.x, pos.y, shape); }

	void drawLineHorizontal(const Position& pos, int width) {
		if (pos.x < 0 || pos.y < 0 || pos.x + width > getWidth() || pos.y > getHeight()) return;
		for (int i = pos.x; i <= min(this->width, pos.x + width); i++) draw(i, pos.y, 178);
	}
	void drawLineVertical(const Position& pos, int height) {
		if (pos.x < 0 || pos.y < 0 || pos.x > getWidth() || pos.y + height > getHeight()) return;
		for (int i = pos.y; i <= min(this->height, pos.y + height); i++) draw(pos.x, i, 178);
	}
	void drawRectangle(const Position& topLeft, const Position& sz) {
		drawLineHorizontal(topLeft, sz.x);
		drawLineHorizontal(topLeft + Position{ 0, sz.y }, sz.x);
		drawLineVertical(topLeft, sz.y);
		drawLineVertical(topLeft + Position{ sz.x, 0 }, sz.y);
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
		FlushConsoleInputBuffer(hStdin);

		string mode = "mode con:cols=" + to_string(Screen::getInstance()->getWidth() + 10);
		mode += " lines=" + to_string(Screen::getInstance()->getHeight() + 5);
		std::system(mode.c_str());
		std::system("chcp 437");

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
	bool GetLeftMouseDown() {
		if (events.empty() == true) return false;
		const INPUT_RECORD& in = events.front();
		if (in.EventType != MOUSE_EVENT) return false;
		return in.Event.MouseEvent.dwEventFlags == 0
			&& (in.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED);
	}

	bool GetRightMouseDown() {
		if (events.empty() == true) return false;
		const INPUT_RECORD& in = events.front();
		if (in.EventType != MOUSE_EVENT) return false;
		return in.Event.MouseEvent.dwEventFlags == 0
			&& (in.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED);
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
			
		for (int i = 0; i < (int)cNumRead; i++) events.push_back(irInBuf[i]);
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

	Position parentPos; // your parent's global position in space
	bool dirty; // mark it TRUE if your local position is changed.
protected:
	Screen& screen;
	InputManager& inputManager;

	vector<GameObject*> children;
	GameObject* parent;
	void setParentPos(const Position& parentPos) { this->parentPos = parentPos; }
public:
	GameObject(int x,int y,const string& shape,GameObject* parent = nullptr)
		:shape(shape),pos(x,y),parent(parent),dirty(false),screen(*Screen::getInstance()),inputManager(*InputManager::getInstance())
	{
		setParentPos(parent ? parent->getWorldPos() : Position{ 0,0 });
		if (parent) parent->add(this);
	}
	GameObject(const Position& pos, const string& shape, GameObject* parent = nullptr)
		: GameObject(pos.x, pos.y, shape,parent) 
	{
	}
	virtual ~GameObject()
	{
	}

	void add(GameObject* child) {
		if (!child) return;
		children.push_back(child);
	}

	void setPos(int x, int y) { setPos(Position{ x, y }); }
	void setPos(const Position& pos) {
		this->pos.x = pos.x; this->pos.y = pos.y;
		dirty = true;
	}
	Position getPos() const { return pos; }
	Position getWorldPos() const { return parentPos + pos; }

	void setParent(GameObject* parent) {
		this->parent = parent;
		setParentPos(parent ? parent->getWorldPos() : Position{ 0,0 });
		for (auto child : children) child->internalUpdatePos(true);
	}

	void internalUpdatePos(bool dirty = false) {
		bool inheritedDirty = dirty;
		if (inheritedDirty == false) {
			if (this->dirty == true) inheritedDirty = true;
		}
		else {
			if (parent) setParentPos(parent->getWorldPos());
		}

		for (auto child : children) child->internalUpdatePos(inheritedDirty);
		this->dirty = false;
	}
	void internalUpdate() {
		update();
		for (auto child : children) child->internalUpdate();
	}
	virtual void update() {}

	void internalDraw() {
		draw();
		for (auto child : children) child->internalDraw();
	}
	virtual void draw() { screen.draw(getWorldPos(), shape.c_str()); }
};

class Block : public GameObject {

public:
	Block(const Position& pos, const string& shape,GameObject* parent = nullptr)
		: GameObject(pos,shape,parent) {}

	void update() override{
		Position pos = getPos();
		if (inputManager.GetKeyDown(VK_DIVIDE)) setPos(pos + Position::left);
		if (inputManager.GetKeyDown(VK_MULTIPLY)) setPos(pos + Position::right);
	}
};

class Panel : public GameObject {
private:
	int width;
	int height;
	
public:
	Panel(const Position pos, int width, int height,GameObject* parent)
		: GameObject(pos, "",parent), width(width), height(height)
	{
	}

	void update() override {
		Position pos = getPos();
		if (inputManager.GetKeyDown(VK_LEFT) == true) setPos(pos + Position::left);
		if (inputManager.GetKeyDown(VK_RIGHT) == true) setPos(pos + Position::right);
		if (inputManager.GetKeyDown(VK_UP) == true) setPos(pos + Position::up);
		if (inputManager.GetKeyDown(VK_DOWN) == true) setPos(pos + Position::down);
	}

	void draw() override{
		Position pos = getWorldPos();
		screen.drawRectangle(Position{ pos.x - 1, pos.y - 1 }, Position{ width + 1, height + 1 });
	}
};

class Text : public GameObject {
public:
	Text(const Position& pos, const string& message, GameObject* parent)
		: GameObject(pos, message.c_str(), parent)
	{}
	void update() override {
	}
};

int main()
{
	Screen& screen = *Screen::getInstance();
	InputManager& inputmanager = *InputManager::getInstance();

	vector<GameObject*> scene;

	auto panel = new Panel{ Position{3,3}, 10, 10, nullptr };
	new Text{ Position{0,3}, "Hello", new Block{ Position{0,0}, "(^_^)", panel } };

	auto panel2 = new Panel{ Position{20, 3}, 10, 10, nullptr };
	new Text{ Position{0,3}, "World",  new Block{ Position{0,0}, "(+_+)", panel2 } };

	scene.push_back(panel);
	scene.push_back(panel2);

	while (true) {
		screen.clear();
	
		inputmanager.readInputs();

		for (auto object : scene) object->internalUpdate();
		for (auto object : scene) object->internalUpdatePos(false);
		for (auto object : scene) object->internalDraw();
	
		screen.render();

		inputmanager.consumeEvent();

		Sleep(100);
	}
	printf("\nGame Over\n");
	return 0;
}
