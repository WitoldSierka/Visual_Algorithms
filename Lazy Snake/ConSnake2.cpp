#include <chrono>
#include <deque>
#include <string>
#include <vector>
#include <Windows.h>

const int screenWidth = 120;
const int screenHeight = 40;
const int screenSize = screenWidth * screenHeight;
int score = 0;
int speed = 10;

int startX = screenWidth / 2.0; // snake starts at the middle of the map
int startY = screenHeight / 2.0;

void snakeBody(const std::pair<int, int> &a, std::pair<int, int> &b);
bool isTimeToMove(std::chrono::system_clock::time_point t, int direction, int timestamp);

int main() {
	// Create Screen Buffer
	std::vector<wchar_t> screen(screenSize);
	//wchar_t* screen = new wchar_t[screenSize];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	bool quit = false;

	while (!quit) {
		std::wstring map;
		bool gameOver = false;
		bool foodEaten = false;
		bool keyLeft = false, keyRight = false, keyLeftOld = false, keyRightOld = false;
		int direction = 0;
		int score = 0;
		int foodY = startY;
		int foodX = screenWidth / 4.0;
		int timestamp = 30;
		std::vector<std::pair<int, int>> obstacles = { };
		std::deque<std::pair<int, int>> snake = {	{startX, startY}, 
													{startX+1, startY}, 
													{startX+2, startY},
													{startX+3, startY}, 
													{startX+4, startY} };
		
		while (!gameOver) {
			//game input and timing
			auto t1 = std::chrono::system_clock::now();
			while (isTimeToMove(t1, direction, timestamp)) {
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
				snake[0].first--;
				break;
			case 1:					// UP
				snake[0].second--;
				break;
			case 2:					// RIGHT
				snake[0].first++;
				break;
			case 3:					// DOWN
				snake[0].second++;
			}

			//rope simulation
			for (std::deque<std::pair<int, int>>::iterator it = snake.begin(); it != snake.end()-1; ++it) {
				snakeBody(*it, *(it + 1));
			}

			foodEaten = false;
			//wall collision
			if (snake.front().first < 0 || snake.front().first >= screenWidth || snake.front().second < 3 || snake.front().second >= screenHeight) {
				snake.pop_front();
				gameOver = true;
			}

			//obstacle collision
			for (const std::pair<int, int>& obs : obstacles) {
				if (snake.front().first == obs.first && snake.front().second == obs.second) gameOver = true;
			}

			//snake collision
			for (std::deque<std::pair<int, int>>::iterator i = snake.begin(); i != snake.end(); ++i) {
				if ( i != snake.begin() && i->first == snake.front().first && i->second == snake.front().second) gameOver = true;
			}

			//food collision 
			if (snake.front().first == foodX && snake.front().second == foodY) {
				// randomly place new food
				srand(unsigned(time(NULL)));
				foodEaten = true;
				foodX = rand() % screenWidth;
				foodY = (rand() % (screenHeight - 3)) + 3;
				score++;
				timestamp--;
				if (timestamp < 1) 
					timestamp == 1;
				if (!(score % 3)) {
					int obsX = rand() % screenWidth;
					int obsY = (rand() % (screenHeight - 3)) + 3;
					obstacles.emplace_back(obsX, obsY);
				}
			}
			if (foodEaten) snake.push_back({snake.back().first, snake.back().second});

			// Clear Screen
			for (int i = 0; i < screenSize; ++i) screen[i] = L' ';

			// Display border and score
			for (int i = 0; i < screenWidth; i++) {
				screen[i] = L'=';
				screen[2 * screenWidth + i] = L'=';
			}
			wsprintf(&screen[screenWidth + 5], L"                L A Z Y   S N A K E ! ! !              SCORE: %d", score * 100);

			// Display Snake: snake body
			for (std::pair<int, int> s : snake) {
				screen[s.first + (s.second * screenWidth)] = gameOver ? L'+' : L'O';
			}
			// snake head
			screen[snake.front().second * screenWidth + snake.front().first] = gameOver? L'X' : L'@';

			//food
			screen[foodY * screenWidth + foodX] = L'A';

			// display obstacles
			for (const std::pair<int, int>& obs : obstacles) {
				screen[obs.second * screenWidth + obs.first] = L'#';
			}

			// game over screen
			if (gameOver) {
				wsprintf(&screen[(screenHeight / 2 - 1) * screenWidth + 40], L"    PRESS 'SPACE' TO PLAY AGAIN    ");
				wsprintf(&screen[(screenHeight / 2    ) * screenWidth + 40], L"    PRESS 'Q' TO CLOSE THE GAME    ");
			}

			//Display frame
			WriteConsoleOutputCharacter(hConsole, screen.data(), screenSize, {0, 0}, &dwBytesWritten);
		}
		//Wait for space to reset
 		while (gameOver) {
			if (0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) {
				gameOver = false;
			} else if (0x8000 & GetAsyncKeyState((unsigned char)('\x51'))) { // Q key
				quit = true;
				break;
			}
		}
	}
    return 0;
}

void snakeBody(const std::pair<int, int> &a, std::pair<int, int> &b) {
	if (abs(a.first - b.first) > 1 || abs(a.second - b.second) > 1) {
		if (a.first > b.first) {
			b.first++;
		} else if (a.first < b.first) {
			b.first--;
		}
		if (a.second > b.second) {
			b.second++;
		} else if (a.second < b.second) {
			b.second--;
		}
	}
}

bool isTimeToMove(std::chrono::system_clock::time_point t, int direction, int timestamp) {
	std::chrono::milliseconds time_amount;
	//if snake is moving vertically time difference must be accounted for, since font tiles are not square
	if (direction & 1)
		time_amount = std::chrono::milliseconds(4 * timestamp);
	else
		time_amount = std::chrono::milliseconds(static_cast<int>(2.5 * timestamp));
	return (std::chrono::system_clock::now() - t) < time_amount;
}