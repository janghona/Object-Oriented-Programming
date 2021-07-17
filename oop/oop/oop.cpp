#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string> // c++ string class
#include <Windows.h>

using namespace std;

const int directionToLeft = 0;
const int directionToRight = 1;

const int screenSize = 80;
const int nBullets = 80;

void clear(char* screen) {
	memset(screen, ' ', screenSize); // clear screen
}

void render(char* screen) {
	screen[screenSize] = '\0';  // render screen
	printf("%s\r", screen);
}

void draw(char* screen,int pos, char face) {
	if(pos < 0 || pos >= screenSize) return;
	screen[pos] = face;
}

void draw(char* screen, int pos, const char* face) {
	strncpy(&screen[pos], face, strlen(face)); // draw
}

void moveLeft(int* pos) {
	--(*pos);
}

void moveRight(int* pos) {
	++(*pos);
}

int findUnusedBullet(bool* isReady) {
	// 총알들중 현재 사용중이지 않은 것을 찾아냄.
	for (int i = 0; i < nBullets; i++)
	{
		if (isReady[i] == true) return i;
	}

	return -1;

}
void fire(bool* isReady, int* bulletPos, int* direction,int playerPos, int enemyPos, char* playerFace ) {
	
	int i = findUnusedBullet(isReady);
	if (i == -1) return;

	// i < nBullets 즉 사용하지 않은 총알이 있습니다.
	isReady[i] = false; // 사용중 표시

	if (playerPos < enemyPos) direction[i] = directionToRight;
	else direction[i] = directionToLeft;

	if (direction[i] == directionToRight) bulletPos[i] = playerPos + strlen(playerFace) - 1;
	else bulletPos[i] = playerPos;
}

void update(char* face,int* nRemaining,const char* target) {
	
	if (*nRemaining > 0) {
		--(*nRemaining);
		if ((*nRemaining) == 0) { // 1초가 지났을 때
			strcpy(face, target);
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
	char screen[screenSize + 1]; // 0 .. 80 : 81

	char playerFace[20]{ "(-_-)" };
	int playerPos = 50;
	int nPlayerRemaining = 0;

	char enemyFace[20]{ "(`_#)" };
	int enemyPos = 10;
	int nEnemyRemaining = 0;

	bool isReady[nBullets];
	int bulletPos[nBullets];
	int direction[nBullets];

	int major;
	int minor;

	initBullets(isReady, bulletPos, direction);

	// game loop
	while (true) {

		clear(screen);

		// handle inputs
		if (_kbhit()) {
			major = _getch();
			switch (major) {
			case ' ':
				fire(isReady, bulletPos, direction, playerPos, enemyPos, playerFace);
				break;
			case 224: // arrow key, function key pressed
				minor = _getch();
				switch (minor) {
				case 75: // left
					moveLeft(&playerPos);
					break;
				case 77: // right
					moveRight(&playerPos);
					break;
				case 72: // up
					break;
				case 80: // down
					break;
				}
				break;
			}
		}

		update(playerFace, &nPlayerRemaining, "(-_-)");
		update(enemyFace, &nEnemyRemaining, "(`_#)");

		// update all bullets
		for (int i = 0;i < nBullets;i++) {
			if (isReady[i] == true) continue;

			// isReady[i] == false
			direction[i] == directionToRight ? bulletPos[i]++ : bulletPos[i]--; // move bullet automatically
			if ((direction[i] == directionToLeft && enemyPos + strlen(enemyFace) - 1 == bulletPos[i])
				|| (direction[i] == directionToRight && enemyPos == bulletPos[i])) { //적이 총알을 맞았을때
				strcpy(enemyFace, "(T_T)");
				nEnemyRemaining = 10;
				strcpy(playerFace, "(^_^)");
				nPlayerRemaining = 30;
				isReady[i] = true;
			}

			if (bulletPos[i] < 0 || bulletPos[i] >= screenSize) {
				isReady[i] = true;
			}
		}
		
		
		draw(screen, playerPos, playerFace);
		draw(screen, enemyPos, enemyFace);

		// draw all bullets in use
		for (int i = 0; i < nBullets; i++)
		{
			if (isReady[i] == true) continue;
			// isReady[i] == false
			if (bulletPos[i] >= 0 && bulletPos[i] < screenSize) {
				draw(screen, bulletPos[i], '-');
			}
		}
		
		render(screen);
		Sleep(100);
	}
	printf("\nGame Over\n");

	return 0;
}