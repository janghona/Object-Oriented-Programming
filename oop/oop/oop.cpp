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

	bool isInRange(Bullet* bullet) { return true; }
};

struct Player{
	char face[20];
	int pos;
	int nRemaining;

	Player(const char* face,int pos) {
		strcpy(this->face, face);
		this->pos = pos;
		this->nRemaining = 0;
	}
	
	void move(int direction){
		(direction == directionToLeft) ? --pos : ++pos;
	}

	int getPos() {return pos;}
	const char* getFace() {return face;}

	void onEnemyHit(){
		strcpy(face, "(^_^)");
		nRemaining = 20;
	}

	void draw(Screen* screen){
		screen->draw(pos, face);
	}

	void update(const char* face){
		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) strcpy(this->face, face);
	}

	void fire(Bullets* bullets, Enemy* enemy) {

		//Bullet* bullet = bullets->find_unused_bullet();
		//if (bullet == nullptr) return;
		//// 사용하지 않은 총알이 있습니다.
		//bullet->isReady = false; // 사용중 표시

		//if (pos < enemy->pos) bullet->direction = directionToRight;
		//else bullet->direction = directionToLeft;

		//if (bullet->direction == directionToRight) bullet->pos = pos + strlen(face) - 1;
		//else bullet->pos = pos;
	}
};

struct Enemy {
	char face[20];
	int pos;
	int nRemaining;

	Enemy(const char* face, int pos) {
		strcpy(this->face, face);
		this->pos = pos;
		this->nRemaining = 0;
	}

	int getPos() {return pos;}

	void onHit() {
		strcpy(face, "(T_T)");
		nRemaining = 10;
	}

	void draw(Screen* screen){
		screen->draw(pos, face);
	}
	void update(const char* face)
	{
		if (nRemaining == 0) return;
		--nRemaining;
		if (nRemaining == 0) strcpy(this->face, face);
	}

	bool isHit(Bullet* bullet) { return true; }
};

struct Bullet{
	bool isReady;
	int pos;
	int direction;

	Bullet() {
		isReady = true;
		pos = 0;
		direction = directionToRight;
	}

	int  getPos(){return pos;}
	int  getDirection(){return direction;}

	void reuse(){
		isReady = true;
	}

	void move(){
		(direction == directionToLeft) ? --pos : ++pos;
	}

	void draw(Screen* screen){
		if (isReady == true) return;
		screen->draw(pos, '-');
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
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			bullet->draw(screen);
		}
	}

	void update(Player* player, Enemy* enemy, Screen* screen){
		for (int i = 0; i < nBullets; i++)
		{
			Bullet* bullet = &bullets[i];
			bullet->update(player, enemy, screen);
		}
	}

	Bullet* find_unused_bullet() {
		// 총알들중 현재 사용중이지 않은 것을 찾아냄.
		for (int i = 0; i < nBullets; i++) {
			Bullet* bullet = &bullets[i];
			if (bullet->isReady == true) return bullet;
		}
		return nullptr;
	}
};

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