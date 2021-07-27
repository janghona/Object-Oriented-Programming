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
struct Bullets;

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

	GameObject(const char* face, int pos,int direction)
		: pos(pos), direction(direction)
	{
		strcpy(this->face, face);
	}

	~GameObject() {}

	int getPos() { return pos; }
	void setPos(int pos) { this->pos = pos; }

	const char* getFace() { return face; }
	void setFace(const char* face) { strcpy(this->face, face); }

	int getDirection() { return direction; }
	void setDirection(int direction) { this->direction = direction; }

	void draw(Screen* screen) {
		screen->draw(pos, face);
	}
};

struct Player : public GameObject{
	int nRemaining;

	Player(const char* face, int pos)
		: GameObject(face, pos, directionToRight), nRemaining(0)
	{ 
	}
	
	void move(int direction){
		(direction == directionToLeft) ? --pos : ++pos;
	}

	void onEnemyHit(){
		strcpy(face, "(^_^)");
		nRemaining = 20;
	}

	void update(const char* face){
		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) strcpy(this->face, face);
	}

	void fire(Bullets* bullets, Enemy* enemy);
};

struct Enemy : public GameObject {
	int nRemaining;

	Enemy(const char* face, int pos)
		: GameObject(face,pos,directionToLeft), nRemaining(0)
	{
	}

	void onHit() {
		setFace("(T_T)");
		nRemaining = 10;
	}

	void update(const char* face)
	{
		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) setFace(face);
	}

	bool isHit(Bullet* bullet);
};

struct Bullet : public GameObject{
	bool isReady;
	
	Bullet()
	:GameObject("-",0,directionToLeft), isReady(true)
	{
	}

	void reuse(){
		isReady = true;
	}

	void move(){
		(direction == directionToLeft) ? --pos : ++pos;
	}

	void update(Player* player, Enemy* enemy, Screen* screen){
		if (isReady == true) return;

		move();
		if (enemy->isHit(this))
		{ // 적이 총알을 맞았을 때
			enemy->onHit();
			player->onEnemyHit();
			reuse();
		}
		if (!screen->isInRange(this)) reuse();
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
};

struct Bullets{
	int nBullets;
	Bullet* bullets;

	Bullets(unsigned int nBullets) {
		if (nBullets == 0) nBullets = 80;
		this->nBullets = nBullets;
		bullets = new Bullet[nBullets];
	}

	~Bullets() {
		delete[] bullets;
		bullets = nullptr;
		nBullets = 0;
	}

	void draw(Screen* screen){
		for (int i = 0; i < nBullets; i++){
			Bullet* bullet = &bullets[i];
			bullet->draw(screen);
		}
	}

	void update(Player* player, Enemy* enemy, Screen* screen){
		for (int i = 0; i < nBullets; i++){
			Bullet* bullet = &bullets[i];
			bullet->update(player, enemy, screen);
		}
	}

	Bullet* find_unused_bullet() {
		for (int i = 0; i < nBullets; i++) {
			Bullet* bullet = &bullets[i];
			if (bullet->isReady == true) return bullet;
		}
		return nullptr;
	}
};

// 전방위 선언 중 사용할 함수
bool Screen::isInRange(Bullet* bullet)
{
	int bullet_pos = bullet->getPos();
	return bullet_pos >= 0 && bullet_pos < size;
}
void Player::fire(Bullets* bullets, Enemy* enemy)
{
	Bullet* bullet = bullets->find_unused_bullet();
	if (bullet == nullptr) return;
	bullet->setFire(this, enemy);
}
bool Enemy::isHit(Bullet* bullet)
{
	int bullet_direction = bullet->getDirection();
	int bullet_pos = bullet->getPos();
	return ((bullet_direction == directionToLeft && getPos() + strlen(getFace()) - 1 == bullet_pos)
		|| (bullet_direction == directionToRight && getPos() == bullet_pos));
}

int main()
{
	Screen screen(80);
	Player player("(-_-)", 50);
	Enemy enemy("('_#)", 10);
	Bullets bullets(80);

	int major;
	int minor;

	// game loop
	while (true) {
		screen.clear();

		player.update("(-_-)");
		enemy.update("(`_#)");
		bullets.update(&player, &enemy, &screen);

		player.draw(&screen);
		enemy.draw(&screen);
		bullets.draw(&screen);
	
		screen.render();
		Sleep(100);

		// handle inputs
		if (!_kbhit()) continue;

			major = _getch();
			switch (major) {
			case ' ':
				player.fire(&bullets,&enemy);
				break;
			case 224: // arrow key, function key pressed
				minor = _getch();
				switch (minor) {
				case 75: // left
					player.move(directionToLeft);
					break;
				case 77: // right
					player.move(directionToRight);
					break;
				case 72: // up
					break;
				case 80: // down
					break;
				}
				break;
		    }
	}
	printf("\nGame Over\n");

	return 0;
}