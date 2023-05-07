#include <chrono>
#include <iostream>
#include <list>
#include <Windows.h>

const int screenWidth = 120;
const int screenHeight = 40;
const int screenSize = screenWidth * screenHeight;
int score = 0;
int speed = 10;

int startX = screenWidth / 2.0; // snake starts at the middle of the map
int startY = screenHeight / 2.0;


int main() {
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[screenSize];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	bool quit = false;

	quit:
	while (!quit) {
		std::wstring map;
		bool gameOver = false;
		bool foodNotEaten = true;
		bool keyLeft = false, keyRight = false, keyLeftOld = false, keyRightOld = false;
		int direction = 0;
		int score = 0;
		int foodY = startY;
		int foodX = screenWidth / 4.0;
		std::list<std::pair<int, int>> snake = { {startX, startY}, {startX+1, startY}, {startX+2, startY}, {startX+3, startY}, {startX+4, startY}, };
		
		while (!gameOver) {
			//game input and timing
			int timestamp = 40;
			auto t1 = std::chrono::system_clock::now();
			while ((std::chrono::system_clock::now() - t1) < ((direction % 2 == 0) ? std::chrono::milliseconds(24 * timestamp /10) : std::chrono::milliseconds(4 * timestamp))) {
				keyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
				keyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
				if (keyRight && !keyRightOld) {
					direction++;
					if (direction > 3) direction = 0;
				}
				if (keyLeft && !keyLeftOld) {
					direction--;
					if (direction < 0) direction = 3;
				}
				keyRightOld = keyRight;
				keyLeftOld = keyLeft;
			}
			// ===================== Game Logic ===================
			switch (direction) {
			case 0:					// LEFT
				snake.push_front({ snake.front().first - 1, snake.front().second });
				break;
			case 1:					// UP
				snake.push_front({ snake.front().first, snake.front().second - 1 });
				break;
			case 2:					// RIGHT
				snake.push_front({ snake.front().first + 1, snake.front().second });
				break;
			case 3:					// DOWN
				snake.push_front({ snake.front().first, snake.front().second + 1 });
			}
			foodNotEaten = true;
			//wall collision
			if (snake.front().first < 0 || snake.front().first >= screenWidth || snake.front().second < 3 || snake.front().second >= screenHeight) {
				snake.pop_front();
				gameOver = true;
			}

			//snake collision
			for (std::list<std::pair<int, int>>::iterator i = snake.begin(); i != snake.end(); ++i) {
				if ( i != snake.begin() && i->first == snake.front().first && i->second == snake.front().second) gameOver = true;
			}

			//food collision 
			if (snake.front().first == foodX && snake.front().second == foodY) {
				// randomly place new food
				foodNotEaten = false;
				foodX = rand() % screenWidth;
				foodY = (rand() % (screenHeight - 3)) + 3;
				score++;
				if (score % 5 == 0) {
					timestamp -= 5;
					if (timestamp < 1) timestamp == 0;
				}
			}
			if (foodNotEaten) snake.pop_back();

			// Clear Screen
			for (int i = 0; i < screenSize; ++i) screen[i] = L' ';

			// Display border and score
			for (int i = 0; i < screenWidth; i++) {
				screen[i] = L'=';
				screen[2 * screenWidth + i] = L'=';
			}
			wsprintf(&screen[screenWidth + 5], L"                          S N A K E ! ! !              SCORE: %d", score * 100);

			// Display Snake: snake body
			for (std::pair<int, int> s : snake) {
				screen[s.first + (s.second * screenWidth)] = gameOver ? L'+' : L'O';
			}
			// snake head
			screen[snake.front().second * screenWidth + snake.front().first] = gameOver? L'X' : L'@';

			//food
			screen[foodY * screenWidth + foodX] = L'A';

			// game over screen
			if (gameOver) wsprintf(&screen[(screenHeight / 2 - 1) * screenWidth + 40], L"    PRESS 'SPACE' TO PLAY AGAIN    ");

			//Display frame
			screen[screenWidth * screenHeight - 1] = '\0';
			WriteConsoleOutputCharacter(hConsole, screen, screenSize, { 0, 0 }, &dwBytesWritten);
		}

		//Wait for space to reset
		while ((0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) == 0);
	}
	delete[] screen;
    return 0;
}
/*
bool snakeCollision(int x, int y) {

}*/