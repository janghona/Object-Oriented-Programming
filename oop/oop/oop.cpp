#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string> // c++ string class
#include <Windows.h>

const int directionToLeft = 0;
const int directionToRight = 1;

struct Screen;
struct Player;
struct Enemy;
struct Bullet;

struct Screen {
	int size;
	char* canvas;

	Screen(unsigned int screenSize) {
		if (size == 0) size = 80;
		size = screenSize;
		canvas = new char[size + 1];
	}

	~Screen() {
		delete[] canvas;
		canvas = nullptr;
		size = 0;
	}

	void clear() {
		memset(canvas, ' ', size); // clear screen
	}

	void render() {
		canvas[size] = '\0';  // render screen
		printf("%s\r", canvas);
	}

	void draw(int pos, char face) {
		if (pos < 0 || pos >= size) return;
		canvas[pos] = face;
	}

	void draw(int pos, const char* face) {
		strncpy(&(canvas[pos]), face, strlen(face)); // for bullet draw
	}

	bool isInRange(Bullet* bullet);
};
struct GameObject {
	char face[20];
	int pos;
	int direction;
	Screen* screen;
	char type[10];
	GameObject** gameObjects;

	GameObject(GameObject** gameObjects,Screen* screen,const char* face, int pos,int direction,const char* type)
		: pos(pos), direction(direction), screen(screen), gameObjects(gameObjects)
	{
		setFace(face);
		strcpy(this->type, type);
	}

	~GameObject() {}

	int getPos() { return pos; }
	void setPos(int pos) { this->pos = pos; }

	const char* getFace() { return face; }
	void setFace(const char* face) { strcpy(this->face, face); }

	int getDirection() { return direction; }
	void setDirection(int direction) { this->direction = direction; }

	Screen* getScreen() { return screen; }
	GameObject** getGameObjects() { return gameObjects; }

	void draw() {
		screen->draw(pos, face);
	}

	void update() {}

	void move(int direction){
		direction == directionToRight ? pos++ : pos--;
	}
	void move(){
		direction == directionToLeft ? --pos : ++pos;
	}

	bool equal(const char* type)
	{
		return strcmp(this->type, type) == 0;
	}

};

struct Player : public GameObject{
	int nRemaining;
	char		originalFace[20];

	Player(GameObject** gameObjects,Screen* screen, const char* face, int pos);
	~Player();

	void draw();
	void update();

	void fire();
	Bullet* find_unused_bullet();
	Enemy* find_enemy();

	void onEnemyHit() {
		setFace("(^_^)");
		nRemaining = 20;
	}
};

struct Enemy : public GameObject {
	int nRemaining;
	char	originalFace[20];

	Enemy(GameObject** gameObjects,Screen* screen,const char* face, int pos)
		: GameObject(gameObjects,screen,face,pos,directionToLeft,"enemy"), nRemaining(0)
	{
		strcpy(originalFace, face);
	}

	void onHit() {
		setFace("(T_T)");
		nRemaining = 10;
	}

	void update()
	{
		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) setFace(originalFace);
	}

	bool isHit(Bullet* bullet);
};

struct Bullet : public GameObject{
	bool isReady;
	
	Bullet(GameObject** gameObjects,Screen* screen)
	:GameObject(gameObjects,screen,"-",0,directionToLeft,"bullet"), isReady(true)
	{
	}

	~Bullet() {}

	void makeReady(){
		isReady = true;
	}

	void update(){
		if (isReady == true) return;

		move();

		GameObject** gameObjects = getGameObjects();
		Player* player = nullptr;
		for (int i = 0;i < 82;i++) {
			GameObject* obj = gameObjects[i];
			if (obj->equal("player")) {
				player = (Player*)obj;
			}
		}

		for (int i = 0; player != nullptr && i < 82;i++) {
			GameObject* obj = gameObjects[i];
			if (obj->equal("enemy")) {
				Enemy* enemy = (Enemy*)obj;
				if (enemy->isHit(this))
				{ // 적이 총알을 맞았을 때
					enemy->onHit();
					player->onEnemyHit();
					makeReady();
				}
			}
		}
		Screen* screen = getScreen();
		if (!screen->isInRange(this)) makeReady();
	}

	void setFire(Player* player, Enemy* enemy) {
		isReady = false; // 사용중ㅇ
		int enemy_pos = enemy->getPos();
		int player_pos = player->getPos();
		const char* player_face = player->getFace();

		setDirection(directionToLeft);
		if (player_pos < enemy_pos) setDirection(directionToRight);

		setPos(player_pos);
		if (getDirection() == directionToRight)
			setPos(getPos() + (strlen(player_face) - 1));
	}

	void draw() {
		if (isReady == true) return;
		GameObject::draw();
	}

	bool isAvailable() { return isReady; }
};

// 전방위 선언 중 사용할 함수
bool Screen::isInRange(Bullet* bullet){
	int bullet_pos = bullet->getPos();
	return bullet_pos >= 0 && bullet_pos < size;
}

Player::Player(GameObject** gameObjects,Screen* screen, const char* face, int pos)
	: GameObject(gameObjects,screen, face, pos, directionToRight,"player"), nRemaining(0)
{
	strcpy(originalFace, face);
}

Player::~Player()
{
}

void Player::fire(){
	Bullet* bullet = find_unused_bullet();
	if (bullet == nullptr) return;
	Enemy* enemy = find_enemy();
	if (enemy == nullptr) return;
	bullet->setFire(this, enemy);
}

Bullet* Player::find_unused_bullet(){
	GameObject** gameObjects = getGameObjects();
	for (int i = 0; i < 82; i++){
		GameObject* obj = gameObjects[i];
		if (obj->equal("bullet") == false) continue;
		Bullet* bullet = (Bullet*)obj;
		if (bullet->isAvailable() == true) return bullet;
	}
	return nullptr;
}

Enemy* Player::find_enemy() {
	GameObject** gameObjects = getGameObjects();
	for (int i = 0; i < 82;i++) {
		GameObject* obj = gameObjects[i];
		if (obj->equal("enemy") == false) continue;
		Enemy* enemy = (Enemy*)obj;
		return enemy;
	}
}

void Player::draw() {
	GameObject::draw();
}

void Player::update() {
	if (nRemaining == 0) return;
	--nRemaining;
	if (nRemaining == 0) setFace(originalFace);
}

bool Enemy::isHit(Bullet* bullet)
{
	int bullet_direction = bullet->getDirection();
	int bullet_pos = bullet->getPos();
	return bullet_pos >= getPos() && bullet_pos < getPos() + strlen(getFace()) - 1;
}

int main()
{
	Screen screen(80);
	GameObject* gameObjects[80 + 1 + 1];

	gameObjects[0] = new Player(gameObjects, &screen, "(-_-)", 50);
	gameObjects[1] = new Enemy(gameObjects, &screen, "(`_#)", 10);

	for (int i = 0; i < 80; i++)
		gameObjects[i + 2] = new Bullet(gameObjects, &screen);

	int major;
	int minor;

	// game loop
	while (true) {
		screen.clear();

		for (int i = 0; i < 82; i++) {
			GameObject* obj = gameObjects[i];
			if (obj->equal("player")) {
				Player* player = (Player*)obj;
				player->update();
			}
			else if (obj->equal("enemy")) {
				Enemy* enemy = (Enemy*)obj;
				enemy->update();
			}
			else if (obj->equal("bullet")) {
				Bullet* bullet = (Bullet*)obj;
				bullet->update();
			}
		}

		for (int i = 0; i < 82; i++) {
			GameObject* obj = gameObjects[i];
			if (obj->equal("player")) {
				Player* player = (Player*)obj;
				player->draw();
			}
			else if (obj->equal("enemy")) {
				Enemy* enemy = (Enemy*)obj;
				enemy->draw();
			}
			else if (obj->equal("bullet")) {
				Bullet* bullet = (Bullet*)obj;
				bullet->draw();
			}
		}
	
		screen.render();
		Sleep(100);

		// handle inputs
		if (!_kbhit()) continue;
			major = _getch();
			switch (major) {
			case ' ':
				((Player*)gameObjects[0])->fire();
				break;
			case 224: // arrow key, function key pressed
				minor = _getch();
				switch (minor) {
				case 75: // left
					gameObjects[0]->move(directionToLeft);
					break;
				case 77: // right
					gameObjects[0]->move(directionToRight);
					break;
				case 72: // up
					break;
				case 80: // down
					break;
				}
				break;
		    }
	}

	for (int i = 0; i < 82; i++)
		delete gameObjects[i];

	printf("\nGame Over\n");
	return 0;
}