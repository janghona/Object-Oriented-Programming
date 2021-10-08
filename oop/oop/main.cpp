#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <ctime>
#include <cstdlib> 
#include <deque>
#include <vector>
#include <string>
#include "Utils.h"
#include "Screen.h"
#include "InputManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "PanelScript.h"
using namespace std;

int main()
{
	Screen& screen = Screen::getInstance();
	InputManager& inputManager = InputManager::getInstance();
	Scene& scene = Scene::getInstance();

	auto mainPanel = new GameObject("mainPanel");
	auto mainPanelScript = new PanelScript(mainPanel);

	auto nextPanel = new GameObject("nextPanel");
	auto nextPanelScript = new PanelScript(nextPanel);

	scene.add(mainPanel);
	scene.add(nextPanel);

	while (true) {
		screen.clear();

		inputManager.readInputs();

		if (inputManager.GetKeyDown(VK_ESCAPE)) break;

		scene.update();
		scene.draw();

		screen.render();

		inputManager.consumeEvent();

		Sleep(100);
	}
	printf("\nTerminated Successfully.\n");
	return 0;
}