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

	GameObject(Screen* screen,const char* face, int pos,int direction)
		: pos(pos), direction(direction), screen(screen)
	{
		setFace(face);
	}

	~GameObject() {}

	int getPos() { return pos; }
	void setPos(int pos) { this->pos = pos; }

	const char* getFace() { return face; }
	void setFace(const char* face) { strcpy(this->face, face); }

	int getDirection() { return direction; }
	void setDirection(int direction) { this->direction = direction; }

	void draw() {
		screen->draw(pos, face);
	}

	void move(int direction){
		direction == directionToRight ? pos++ : pos--;
	}
	void move(){
		direction == directionToLeft ? --pos : ++pos;
	}
};

struct Player : public GameObject{
	int nRemaining;
	int nBullets;
	Bullet** bullets;
	char		originalFace[20];

	Player(Screen* screen, const char* face, int pos);
	~Player();

	void draw();
	void update(Enemy* enemy);

	void fire(Enemy* enemy);
	Bullet* find_unused_bullet();

	void onEnemyHit() {
		setFace("(^_^)");
		nRemaining = 20;
	}
};

struct Enemy : public GameObject {
	int nRemaining;
	char	originalFace[20];

	Enemy(Screen* screen,const char* face, int pos)
		: GameObject(screen,face,pos,directionToLeft), nRemaining(0)
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
	
	Bullet(Screen* screen)
	:GameObject(screen,"-",0,directionToLeft), isReady(true)
	{
	}

	~Bullet() {}

	void makeReady(){
		isReady = true;
	}

	void update(Player* player, Enemy* enemy){
		if (isReady == true) return;

		move();

		if (enemy->isHit(this))
		{ // 적이 총알을 맞았을 때
			enemy->onHit();
			player->onEnemyHit();
			makeReady();
		}
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
bool Screen::isInRange(Bullet* bullet)
{
	int bullet_pos = bullet->getPos();
	return bullet_pos >= 0 && bullet_pos < size;
}

Player::Player(Screen* screen, const char* face, int pos)
	: GameObject(screen, face, pos, directionToRight), nRemaining(0),
	nBullets(80), bullets( new Bullet*[nBullets])
{
	for (int i = 0; i < nBullets; i++){
		bullets[i] = new Bullet(screen);
	}
	strcpy(originalFace, face);
}

Player::~Player()
{
	for (int i = 0; i < nBullets; i++)
		delete bullets[i];
	delete[] bullets;
	bullets = nullptr;
	nBullets = 0;
}

void Player::fire(Enemy* enemy)
{
	Bullet* bullet = find_unused_bullet();
	if (bullet == nullptr) return;
	bullet->setFire(this, enemy);
}

Bullet* Player::find_unused_bullet()
{
	for (int i = 0; i < nBullets; i++){
		Bullet* bullet = bullets[i];
		if (bullet->isAvailable() == true) return bullet;
	}
	return nullptr;
}

void Player::draw() {
	GameObject::draw();
	for (int i = 0; i < nBullets;i++) {
		Bullet* bullet = bullets[i];
		bullet->draw();
	}
}

void Player::update(Enemy* enemy) {
	for (int i = 0; i < nBullets; i++){
		Bullet* bullet = bullets[i];
		bullet->update(this,enemy);
	}

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
	Player* player = new Player(&screen,"(-_-)", 50);
	Enemy* enemy = new Enemy(&screen,"(`_#)", 10);

	int major;
	int minor;

	// game loop
	while (true) {
		screen.clear();

		player->update(enemy);
		enemy->update();

		player->draw();
		enemy->draw();
	
		screen.render();
		Sleep(100);

		// handle inputs
		if (!_kbhit()) continue;

			major = _getch();
			switch (major) {
			case ' ':
				player->fire(enemy);
				break;
			case 224: // arrow key, function key pressed
				minor = _getch();
				switch (minor) {
				case 75: // left
					player->move(directionToLeft);
					break;
				case 77: // right
					player->move(directionToRight);
					break;
				case 72: // up
					break;
				case 80: // down
					break;
				}
				break;
		    }
	}
	delete enemy;
	delete player;

	printf("\nGame Over\n");

	return 0;
}