﻿#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstdlib> // stdlib.h
#include <Windows.h>
#include<deque>
#include<vector>
#include"Utils.h"
using namespace std;

class MyString {
private:
	char* data;
	int capacity;
	
	void resize(int capacity) {
		if (this->capacity >= capacity) return;
		char* newRegion = new char[capacity];
		strcpy(newRegion, data);
		delete[] data;
		data = newRegion;
		this->capacity = capacity;
	}
	MyString& append(const char* str) {
		if (str == nullptr) return *this;
		int len = strlen(data) + strlen(str) + 1;
		while (this->capacity < len) resize(2 * this->capacity);
		strcpy(&data[strlen(data)], str);
		return *this;
	}
public:
	MyString(const char* str, int capacity = 128)
		: data(new char[capacity]), capacity(capacity)
	{
		int len = strlen(str) + 1;
		while (this->capacity < len) resize(2 * this->capacity);
		if (str == nullptr) {
			data[0] = '\0';
			return;
		}
		strcpy(data, str);
	}
	MyString(const MyString& other)
		:MyString(other.data, max(128, strlen(other.data) + 1)) {}
	~MyString() {
		if (data) delete[] data;
	}

	MyString operator+(const MyString& str) { return *this + str.c_str(); }
	MyString operator+(const char* str) { return MyString{ *this }.append(str); }
	MyString& operator+=(const MyString& str) {
		this->append(str.c_str());
		return *this;
	}

	friend MyString operator+(const char* str, const MyString& other);
	friend MyString to_mystring(int value);

	const char* c_str() const { return data; }
};

MyString operator+(const char* str, const MyString& other) {
	return MyString{ str } +other;
}
MyString to_mystring(int value) {
	char buffer[11];
	sprintf(buffer, "%d", value);
	return MyString(buffer);
}

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
			instance = new Screen(80,30);
		}
		return instance;
	}
	~Screen() { delete[] buffer; }

	int getWidth() const { return width; }
	int getScreenWidth() const { return width + 1; }
	int getHeight() const { return height; }
	int getSize() const { return getScreenWidth() * height; }

	void clear() {
		memset(buffer, ' ', getSize());
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
	void draw(int x, int y, const char* shape, int len) {
		if (shape == nullptr || len == 0 || len > strlen(shape)) return;
		strncpy(&buffer[y* getScreenWidth() + x], shape, len);
	}
	void draw(const Position& pos, char shape) { draw(pos.x, pos.y, shape); }
	void draw(const Position& pos, const char* shape) { draw(pos.x, pos.y, shape,strlen(shape)); }
	void draw(const Position& pos, const char* shape, int len) { draw(pos.x, pos.y, shape, len); }

	void drawLineHorizontal(const Position& pos, int width) {
		if (pos.x < 0 || pos.y < 0 || pos.x + width > getWidth() || pos.y > getHeight()) return;
		for (int i = pos.x; i <= min(this->width, pos.x + width); i++) draw(i, pos.y, 196);
	}
	void drawLineVertical(const Position& pos, int height) {
		if (pos.x < 0 || pos.y < 0 || pos.x > getWidth() || pos.y + height > getHeight()) return;
		for (int i = pos.y; i <= min(this->height, pos.y + height); i++) draw(pos.x, i, 179);
	}
	void drawRectangle(const Position& topLeft, const Position& sz) {
		drawLineHorizontal(topLeft, sz.x);
		drawLineHorizontal(topLeft + Position{ 0, sz.y }, sz.x);
		drawLineVertical(topLeft, sz.y);
		drawLineVertical(topLeft + Position{ sz.x, 0 }, sz.y);
		draw(topLeft, 218); draw(topLeft + Position{ sz.x, 0 }, 191);
		draw(topLeft + Position{ 0, sz.y }, 192); draw(topLeft + sz, 217);
	}
	void drawShape(const Position& pos, const Position& sz, const char* shape){
		if (shape == nullptr) return;
		for (int i = 0; i < sz.y; i++) draw(pos.x, pos.y + i, &shape[i*sz.x], sz.x);
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

		MyString mode = "mode con:cols=" + to_mystring(Screen::getInstance()->getWidth() + 10);
		mode += " lines=" + to_mystring(Screen::getInstance()->getHeight() + 5);
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
		INPUT_RECORD in = events.front();
		if (in.EventType != KEY_EVENT) return false;
		if (in.Event.KeyEvent.bKeyDown = TRUE) {
			return in.Event.KeyEvent.wVirtualKeyCode == ch;
		}
		return false;
	}

	Position mousePosition; // it will be used only when a mouse event occurs.
	bool GetLeftMouseDown() {
		if (events.empty() == true) return false;
		const INPUT_RECORD in = events.front();
		if (in.EventType != MOUSE_EVENT) return false;
		mousePosition.x = in.Event.MouseEvent.dwMousePosition.X;
		mousePosition.y = in.Event.MouseEvent.dwMousePosition.Y;
		return in.Event.MouseEvent.dwEventFlags == 0
			&& (in.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED);
	}

	bool GetRightMouseDown() {
		if (events.empty() == true) return false;
		const INPUT_RECORD in = events.front();
		if (in.EventType != MOUSE_EVENT) return false;
		mousePosition.x = in.Event.MouseEvent.dwMousePosition.X;
		mousePosition.y = in.Event.MouseEvent.dwMousePosition.Y;
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
	MyString shape;
	Position pos;

	Position parentPos; // your parent's global position in space
	bool dirty; // mark it TRUE if your local position is changed.

	bool hidden;
protected:
	Screen& screen;
	InputManager& inputManager;

	vector<GameObject*> children;
	GameObject* parent;
	void setParentPos(const Position& parentPos) { this->parentPos = parentPos; }
public:
	GameObject(int x,int y,const MyString& shape,GameObject* parent = nullptr,bool hidden=false)
		:shape(shape),pos(x,y),parent(parent),dirty(false),hidden(hidden),screen(*Screen::getInstance()),inputManager(*InputManager::getInstance())
	{
		setParentPos(parent ? parent->getWorldPos() : Position{ 0,0 });
		if (parent) parent->add(this);
	}
	GameObject(const Position& pos, const MyString& shape, GameObject* parent = nullptr,bool hidden=false)
		: GameObject(pos.x, pos.y, shape,parent,hidden) 
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
	const char* getShape() const { return shape.c_str(); }
	void setShape(const MyString& shape) { this->shape = shape; }

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
		if (hidden == true) return;
		draw();
		for (auto child : children) child->internalDraw();
	}
	virtual void draw() { screen.draw(getWorldPos(), shape.c_str()); }

	// answer 2
	bool isHidden() const { return hidden; }
	void setHidden() { hidden = true; }
	void setVisible() { hidden = false; }

	// answer 3
	virtual bool isOverlapped(const Position& position, const Position& size = Position{ 1,1 }) const {
		Position pos = getWorldPos();
		return pos.x == position.x && pos.y == position.y;
	}
};

// we need to change the following as class SceneManager applying Singleton pattern
vector<GameObject*> scene;

// answer 4
class Clickable : public GameObject {
	Position size;

public:
	Clickable(const MyString& shape, const Position& pos, const Position& sz, GameObject* parent = nullptr, bool hidden = false)
		: GameObject(pos, shape, parent, hidden), size(sz)
	{}

	void update() override {
		if (inputManager.GetLeftMouseDown()) {
			if (isOverlapped(inputManager.mousePosition)) {
				onClick();
			}
		}
	}

	bool isOverlapped(const Position& position, const Position& sz = Position{ 1,1 }) const override {
		Position pos = getWorldPos();
		return (pos.x <= position.x && position.x < pos.x + size.x)
			&& (pos.y <= position.y && position.y < pos.y + size.y);
	}

	virtual void onClick() {}
};

class RestoreButton : public Clickable {
	GameObject* associated;
public:
	RestoreButton(const MyString& shape, const Position& pos, GameObject* panel, bool hidden = false)
		: Clickable(shape, pos, Position{ 10, 1 }, nullptr, hidden), associated(panel)
	{}

	void onClick() override {
		if (associated == nullptr) return;
		associated->setVisible();
		setHidden();
	}
	bool isMatching(GameObject* associated) { return associated == this->associated; }

	void show() {
		setVisible();
		int numberOfRestoreButtons = 0;
		for (auto obj : scene) {
			auto restoreButton = dynamic_cast<RestoreButton *>(obj);
			if (restoreButton == nullptr) continue;
			if (restoreButton->isHidden() == false) numberOfRestoreButtons++;
		}
		this->setPos(screen.getWidth() - 9, screen.getHeight() - 2 - numberOfRestoreButtons);
	}
};

class MinimizeIcon : public Clickable {
public:
	MinimizeIcon(const Position& pos, GameObject* parent, bool hidden = false)
		: Clickable("\xB0", pos, Position{ 1,1 }, parent, hidden)
	{}

	void onClick() override {
		if (parent == nullptr) return;
		parent->setHidden();

		for (auto obj : scene) {
			auto restore = dynamic_cast<RestoreButton*>(obj);
			if (restore == nullptr) continue;
			if (restore->isMatching(this->parent)) {
				restore->show();
			}
		}
	}
};


class Block : public GameObject {
private:
	Position size;
	char* internalShapeData; // it is used only in rotateShape().
	bool rotatable;

	void rotateShape() {
		if (rotatable == false) return;

		string shape = getShape();
		int w = size.x; int h = size.y;
		const char* p = shape.c_str();
		strncpy(internalShapeData, p, w*h);
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				internalShapeData[(w - 1 - x)*h + y] = p[y*w + x];
		setShape(internalShapeData);
		size.x = h; size.y = w;
	}
public:
	Block(const Position& pos, const MyString& shape,const Position& size,GameObject* parent = nullptr,bool rotatable = true)
		: GameObject(pos,shape,parent),size(size), internalShapeData(new char[size.x*size.y + 1]),rotatable(rotatable) 
	{}
	~Block() {
		if (internalShapeData != nullptr) delete[] internalShapeData;
	}
	void update() override{
		Position pos = getPos();
		if (inputManager.GetKeyDown(VK_UP)) rotateShape();
	}
	void draw() override{
		Position pos = getWorldPos();
		screen.drawShape(pos, size, getShape());
	}
};

class Panel : public GameObject {
private:
	int width;
	int height;
	MyString title;
public:
	Panel(const MyString& title,const Position pos, int width, int height,GameObject* parent)
		: GameObject(pos, "",parent), width(width), height(height),title(title)
	{
		auto minimizeIcon = new MinimizeIcon(Position{ width, -1 }, this, false);
		children.push_back(minimizeIcon);

		int numberOfRestoreButtons = 0;
		for (auto obj : scene) {
			auto restoreButton = dynamic_cast<RestoreButton *>(obj);
			if (restoreButton == nullptr) continue;
			numberOfRestoreButtons++;
		}
		scene.push_back(new RestoreButton(MyString{ "R " + to_mystring(numberOfRestoreButtons) },
			Position{ 0, numberOfRestoreButtons }, this, true));
	}

	void update() override {
		Position pos = getPos();
	}

	void draw() override{
		Position pos = getWorldPos();
		screen.drawRectangle(Position{ pos.x - 1, pos.y - 1 }, Position{ width + 2, height + 2 });
		screen.draw(Position{ pos.x + 1, pos.y - 1 }, title.c_str());
	}
};

class Text : public GameObject {
public:
	Text(const Position& pos, const MyString& message, GameObject* parent)
		: GameObject(pos, message.c_str(), parent)
	{}
	void update() override {
	}
};

class TextInput : public GameObject {
private:
	int value;
public:
	TextInput(const Position& pos, int data, GameObject* parent)
		: GameObject(pos, "", parent), value(data)
	{}

	void update() override{
		Position pos = getPos();
	}

	void draw() override{
		static char buffer[10];
		sprintf(buffer, "%3d\0", value);
		Position pos = getWorldPos();
		screen.draw(pos, buffer);
	}

	void setValue(int value) { this->value = value; }
};

int main()
{
	Screen& screen = *Screen::getInstance();
	InputManager& inputmanager = *InputManager::getInstance();

	//vector<GameObject*> scene;

	auto panel = new Panel{ "",Position{3,3}, 10, 20, nullptr };
	new Block{ Position{4,0}, "\xdb  \xdb\xdb\xdb  \xdb", Position{ 3, 3},  panel };
	new Block{ Position{10,0}, "\xdb\xdb \xdb\xdb\xdb", Position{ 2, 3},  panel };

	auto nextPanel = new Panel{ " Next", Position{20, 3}, 10, 5, nullptr };
	new Block{ Position{5, 1}, "\xdb \xdb \xdb\xdb", Position{ 2, 3 }, nextPanel, false };

	auto scorePanel = new Panel{ " Score", Position{20, 19}, 10, 4, nullptr };
	int value = 0;
	auto score = new TextInput{ Position{4, 2}, value, scorePanel };

	scene.push_back(panel);
	scene.push_back(nextPanel);
	scene.push_back(scorePanel);

	while (true) {
		screen.clear();
	
		inputmanager.readInputs();

		if (inputmanager.GetKeyDown(VK_SPACE)) {
			value++;
			score->setValue(value);
		}
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
