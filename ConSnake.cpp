#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>
#include <Windows.h>

void displayStatistic(float time, const std::wstring map, wchar_t* screen);

int nScreenWidth = 120;
int nScreenHeight = 40;

float playerX = 8.0f;
float playerY = 8.0f;
float playerA = 0.0f; // angle at which the player is looking

int mapHeight = 16;
int mapWidth = 16;

float FOV = 3.14159 / 4.0f;
float depth = 16.0f;

int main() {
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	std::wstring map;

	map += L"################"; //1
	map += L"#...#..........#"; //2
	map += L"#...#..#.......#";
	map += L"#...#..........#";
	map += L"#..............#"; //5
	map += L"#.........#....#";
	map += L"#......####....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#"; //10
	map += L"########.......#";
	map += L"#..............#";
	map += L"#........#######";
	map += L"#.#............#";
	map += L"#.##...........#"; //15
	map += L"################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (1) {
		//Timing
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed = tp2 - tp1;
		tp1 = tp2;
		float elapsedTime = elapsed.count();

		// Controls & keyboard handling
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) { playerA -= 0.8f * elapsedTime; }
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) { playerA += 0.8f * elapsedTime; }
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			playerX += sinf(playerA) * 5.0f * elapsedTime;
			playerY += cosf(playerA) * 5.0f * elapsedTime;
			//Collision detection
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX -= sinf(playerA) * 5.0f * elapsedTime;
				playerY -= cosf(playerA) * 5.0f * elapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) { 
			playerX -= sinf(playerA) * 5.0f * elapsedTime;
			playerY -= cosf(playerA) * 5.0f * elapsedTime;
			//Collision detection
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX += sinf(playerA) * 5.0f * elapsedTime;
				playerY += cosf(playerA) * 5.0f * elapsedTime;
			}
		}

		// raycaster
		for (int x = 0; x < nScreenWidth; x++) {
			// for each column, calculate the casted ray angle into world space
			float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)nScreenWidth) * FOV;

			float distanceToWall = 0;
			bool hitWall = false;
			bool hitBoundary = false; // flag for hitting corners of a map block

			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth) {
				distanceToWall += 0.1f;
				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);
				// Test if ray is out of bounds
				if (testX < 0 || testY >= mapWidth || testY < 0 || testY >= mapHeight) {
					hitWall = true;				//set distance to maximum depth
					distanceToWall = depth;
				} else {
					if (map[testY * mapWidth + testX] == '#') {
						hitWall = true;
						std::vector<std::pair<float, float>> p; // distance, dot product
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)testY + ty - playerY;
								float vx = (float)testX + tx - playerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (eyeX * vx / d) + (eyeY * vy / d);
								p.push_back(std::make_pair(d, dot));
							}
						}
						//Sort pairs from closest to farthest
						std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) { return left.first < right.first; });
						float bound = 0.008f;
						if (distanceToWall > 4.5f) { bound = 0.004f; }
						if (acos(p.at(0).second) < bound) hitBoundary = true;
						if (acos(p.at(1).second) < bound) hitBoundary = true;
						//if (acos(p.at(2).second) < bound) hitBoundary = true;
					}
				}
			}
			// Calculate distance to ceiling and floor
			int ceiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)distanceToWall);
			int floor = nScreenHeight - ceiling;

			//Shading using extended ASCII list
			short shade = ' ';
			if (distanceToWall <= depth / 4.0f)     { shade = 0x2588; } // very close
			else if (distanceToWall < depth / 3.0f) { shade = 0x2593; }
			else if (distanceToWall < depth / 2.0f) { shade = 0x2592; }
			else if (distanceToWall < depth)		{ shade = 0x2591; }
			else									{ shade = ' '; }    //too far away

			if (hitBoundary) { shade = ' '; }

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= ceiling) {
					screen[y * nScreenWidth + x] = ' ';
				} else if (y > ceiling && y <= floor) {
					screen[y * nScreenWidth + x] = shade;
				} else {
					//Shade floor based on distance
					float floorDist = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					short floorShade = ' ';
					if (floorDist < 0.25)		{ floorShade = '#'; }
					else if (floorDist < 0.5)	{ floorShade = 'x'; }
					else if (floorDist < 0.75)	{ floorShade = '-'; }
					else if (floorDist < 0.9)	{ floorShade = '.'; }
					//else						{ shade = ' '; }
					screen[y * nScreenWidth + x] = floorShade;
				}
			}
		}
		displayStatistic(elapsedTime, map, screen);
		//Display frame
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}
	delete[] screen;
    return 0;
}

void displayStatistic(float time, const std::wstring map, wchar_t *screen) {
	float fps = 1.0f / time;
	// Display Stats, Map, Player Location
	swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%4.2f ", playerX, playerY, playerA, fps);
	for (int nx = 0; nx < mapWidth; nx++) {
		for (int ny = 0; ny < mapHeight; ny++) {
			screen[(ny + 1) * nScreenWidth + nx] = map[ny * mapWidth + (mapWidth - nx - 1)];
		}
	}
	screen[((int)playerY+1) * nScreenWidth + (int)(mapWidth - playerX)] = 'P';
}