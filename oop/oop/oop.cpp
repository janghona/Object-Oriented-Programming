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

const int screenSize = 80;
const int nBullets = 80;

struct Screen {
	char canvas[screenSize + 1]; // 0 .. 80 : 81
};

struct Character {
	char face[20];
	int pos;
	int nRemaining;

	void init(const char* face,int pos) {
		strcpy(this->face, face);
		this->pos = pos;
		this->nRemaining = 0;
	}
};

void clear(Screen* screen) {
	memset(screen, ' ', screenSize); // clear screen
}

void render(Screen* screen) {
	screen->canvas[screenSize] = '\0';  // render screen
	printf("%s\r", screen->canvas);
}

void draw(Screen* screen,int pos, char face) {
	if(pos < 0 || pos >= screenSize) return;
	screen->canvas[pos] = face;
}

void draw(Screen* screen, int pos, const char* face) {
	strncpy(&(screen->canvas[pos]), face, strlen(face)); // for bullet draw
}

void moveLeft(Character* player) {
	//--(*pos);
	(player->pos)--;
}

void moveRight(Character* player) {
	//++(*pos);
	(player->pos)++;
}

int findUnusedBullet(bool* isReady) {
	// 총알들중 현재 사용중이지 않은 것을 찾아냄.
	for (int i = 0; i < nBullets; i++)
	{
		if (isReady[i] == true) return i;
	}

	return -1;

}
void fire(bool* isReady, int* bulletPos, int* direction,Character* player, Character* enemy) {
	
	int i = findUnusedBullet(isReady);
	if (i == -1) return;

	// i < nBullets 즉 사용하지 않은 총알이 있습니다.
	isReady[i] = false; // 사용중 표시

	if (player->pos < enemy->pos) direction[i] = directionToRight;
	else direction[i] = directionToLeft;

	if (direction[i] == directionToRight) bulletPos[i] = player->pos + strlen(player->face) - 1;
	else bulletPos[i] = player->pos;
}

void update(Character* character,const char* target) {
	
	if (character->nRemaining > 0) {
		--(character->nRemaining);
		if (character->nRemaining == 0) { // 1초가 지났을 때
			strcpy(character->face, target);
		}
	}
}

void initBullets(bool* isReady, int* bulletPos,int* direction) {
	// initialize bullets
	for (int i = 0; i < nBullets; i++)
	{
		isReady[i] = true;
		bulletPos[i] = 0;
		direction[i] = directionToRight;
	}
}

int main()
{
	Screen screen;
	Character player;
	Character enemy;

	player.init("(-_-)", 30);
	enemy.init("('_#)", 60);

	bool isReady[nBullets];
	int bulletPos[nBullets];
	int direction[nBullets];

	int major;
	int minor;

	initBullets(isReady, bulletPos, direction);

	// game loop
	while (true) {

		clear(&screen);

		// handle inputs
		if (_kbhit()) {
			major = _getch();
			switch (major) {
			case ' ':
				fire(isReady, bulletPos, direction, &player, &enemy);
				break;
			case 224: // arrow key, function key pressed
				minor = _getch();
				switch (minor) {
				case 75: // left
					moveLeft(&player);
					break;
				case 77: // right
					moveRight(&player);
					break;
				case 72: // up
					break;
				case 80: // down
					break;
				}
				break;
			}
		}

		update(&player, "(-_-)");
		update(&enemy, "(`_#)");

		// update all bullets
		for (int i = 0;i < nBullets;i++) {
			if (isReady[i] == true) continue;

			// isReady[i] == false
			direction[i] == directionToRight ? bulletPos[i]++ : bulletPos[i]--; // move bullet automatically
			if ((direction[i] == directionToLeft && enemy.pos + strlen(enemy.face) - 1 == bulletPos[i])
				|| (direction[i] == directionToRight && enemy.pos == bulletPos[i])) { //적이 총알을 맞았을때
				strcpy(enemy.face, "(T_T)");
				enemy.nRemaining = 20;
				strcpy(player.face, "(^_^)");
				player.nRemaining = 20;
				isReady[i] = true;
			}

			if (bulletPos[i] < 0 || bulletPos[i] >= screenSize) {
				isReady[i] = true;
			}
		}
		
		
		draw(&screen, player.pos, player.face);
		draw(&screen, enemy.pos, enemy.face);

		// draw all bullets in use
		for (int i = 0; i < nBullets; i++)
		{
			if (isReady[i] == true) continue;
			// isReady[i] == false
			if (bulletPos[i] >= 0 && bulletPos[i] < screenSize) {
				draw(&screen, bulletPos[i], '-');
			}
		}
		
		render(&screen);
		Sleep(100);
	}
	printf("\nGame Over\n");

	return 0;
}