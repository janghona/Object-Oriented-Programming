#pragma once
#include <string>
#include <vector>
using namespace std;

class Screen;
class Scene;
class Component;
class Transform;
class InputManager;


class GameObject {
private:
	string		name;
	string		tag;
	bool		hideFlag;
protected:
	Screen& screen;
	Scene& scene;
	InputManager& inputManager;
	Transform*	transform;

	vector<Component*> components;
	vector<GameObject*> children;
	GameObject* parent;

	friend class Component;
	friend class Scene;

	void internalUpdate();
	void internalDraw();
public:
	GameObject(const string& name, const string& tag = "Unknown",GameObject* parent = nullptr);
	~GameObject();

	void addComponent(Component* comp);

	const string getName() const { return name; }
};