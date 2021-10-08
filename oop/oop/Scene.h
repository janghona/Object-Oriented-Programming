#pragma once
#include <iostream>
#include <vector>
using namespace std;

class GameObject;

class Scene {
	vector<GameObject *> gameObjects;
	Scene();
public:
	static Scene& getInstance();

	void add(GameObject* go);

	void remove(GameObject* go);

	void update();

	void draw();
};