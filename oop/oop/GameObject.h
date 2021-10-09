#pragma once
#include <string>
#include <vector>
#include "Utils.h"
#include "Transform.h"
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

	GameObject(const string& name, const string& tag,GameObject* parent);
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

	void internalStart();
	void internalUpdate();
	void internalDraw();

	static GameObject* Instantiate(const string& name,
		const string& tag = "Unknown",
		GameObject* parent = nullptr,
		const Position& pos = Position{ 0, 0 },
		const string& shape = "",
		const Position& size = Position{1,1}
	);

public:
	~GameObject();

	template<typename T>
	void addComponent()
	{
		auto t = new T(this);
		if (dynamic_cast<Component *>(t) == nullptr) {
			delete t;
			return;
		}
		components.push_back(t);
	}

	template<typename T>
	T* getComponent()
	{
		for (auto component : components) {
			auto found = dynamic_cast<T *>(component);
			if (found) return found;
		}
		return nullptr;
	}

	template<typename T>
	T* getOrAddComponent()
	{
		auto t = getComponent<T>();
		if (t == nullptr) {
			addComponent<T>();
			t = getComponent<T>();
		}
		return t;
	}

	template<typename T>
	void removeComponent()
	{
		for (auto it = components.begin(); it != components.end(); )
		{
			if (dynamic_cast<T *>(*it)) it = components.erase(it);
			else it++;
		}
	}

	void add(GameObject* child) {
		if (child == nullptr) return;
		children.push_back(child);
	}

	const string getName() const { return name; }
};