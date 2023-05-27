#include <algorithm>
#include <chrono>
#include <math.h>
#include <string>
#include <vector>
#include <Windows.h>

constexpr float PI = 3.14159262f;
constexpr float _2PI = 2 * PI;
constexpr float D360 = 2 * PI;
constexpr float D270 = 3 * PI / 2;
constexpr float D180 = PI;
constexpr float D90 = PI / 2;

int screenWidth = 600;
int screenHeight = 300;
size_t screenSize = screenWidth * screenHeight;
short fontWidth = 2;
short fontHeight = 2;
int mapHeight = 16;
int mapWidth = 16;
float FOV = PI / 3;

class Player {
public:
	float A = 0;
	float X = 1.5;
	float Y = 1.5;
};

enum PIXEL_RANGE {
	FULL = 0x2588,
	O75 = 0x2593,
	O50 = 0x2592,
	O25 = 0x2591
};

void clearScreen(std::vector<CHAR_INFO>& s);
int createConsole(SMALL_RECT& w, HANDLE& h);
float dist(float ax, float ay, float bx, float by, float angle);
void draw_map(std::vector<CHAR_INFO>& s, const std::vector<int>& m, size_t mw, size_t mh, size_t sw, size_t ps = 1);
std::vector<int> map_parser(std::vector<int> inMap, int mWidth, int mHeight);

std::vector<int> test_map{ {
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
		3,0,3,3,3,3,3,3,3,3,3,3,0,3,3,3,
		3,0,3,0,0,0,0,0,0,0,0,0,0,0,0,1,
		3,0,3,0,1,0,0,0,0,0,0,0,0,0,0,1,
		3,0,3,0,1,0,0,0,1,0,0,0,0,1,1,1,
		3,0,3,0,1,0,1,0,1,0,0,0,1,0,0,1,
		3,0,3,0,1,0,0,0,0,0,0,0,1,0,0,1,
		3,0,3,0,0,0,0,0,0,0,0,0,0,0,0,1,
		3,0,3,1,1,1,1,1,0,0,0,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,15,15,15,0,0,1,
		1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	} };
// - - - - COLORS OF THE WINDOW CONSOLE - - - -
// 1 - blue
// 2 - green
// 3 - light blue
// 4 - red
// 5 - purple
// 6 - dirty yellow
// 7 - light gray
// 8 - dark grey
// 9 - pastel? blue
// 10 or 0xA - bright green
// 11 or 0xB - cyan
// 12 or 0xC - rose? pink (lively)
// 13 or 0xD - bright pink
// 14 or 0xE - beige
// 15 or 0xF - white
// 16 or 0x10 - very dark (ink) blue
// 17 or 0x11 - full (background) blue. 0x00T0 - T position represents background colour
// 

std::vector<int> map = map_parser(test_map, mapWidth, mapHeight);

int main() {
	std::vector<CHAR_INFO> screen(screenSize);
	memset(screen.data(), 0, sizeof(CHAR_INFO) * screenSize);
	for (size_t i = 0; i < screenSize; i++) {
		screen[i].Char.UnicodeChar = 0x2588;
	}
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT windowSize = { 0, 0, 1, 1 };
	int result = createConsole(windowSize, console);
	if (result) { return result; }
	bool gameOver = false;
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	Player p;

	while (!gameOver) {
		clearScreen(screen);
		// - - - - - TIMING - - - - - -
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed = tp2 - tp1;
		tp1 = tp2;
		float elapsedTime = elapsed.count();

		// - - - - - CONTROLS - - - - -
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) { 
			p.A += 1.3f * elapsedTime;
			if (p.A > _2PI) { p.A -= _2PI; }
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) { 
			p.A -= 1.3f * elapsedTime;
			if (p.A < 0) { p.A += _2PI; }
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			p.X += cosf(p.A) * elapsedTime * 5.0f;
			p.Y += sinf(p.A) * elapsedTime * 5.0f;
			//collision detection
			if (map[int(p.Y) * mapWidth + int(p.X)] > 0) {
				p.X -= cosf(p.A) * elapsedTime * 5.0f;
				p.Y -= sinf(p.A) * elapsedTime * 5.0f;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			p.X -= cosf(p.A) * elapsedTime * 5.0f;
			p.Y -= sinf(p.A) * elapsedTime * 5.0f;
			//collision detection
			if (map[int(p.Y) * mapWidth + int(p.X)] > 0) {
				p.X += cosf(p.A) * elapsedTime * 5.0f;
				p.Y += sinf(p.A) * elapsedTime * 5.0f;
			}
		}
		// - - - - - RAYCASTING - - - - -
		float ROV = 16.0f; //range of view

		float dx = ceil(p.X) - p.X; // X offset within tile
		float d2x = p.X - floor(p.X);
		float dy = ceil(p.Y) - p.Y; // Y offset within tile
		float d2y = p.Y - floor(p.Y);
		int mapPos = 0;
		for (int x = 0; x < screenWidth; x++) {
			int tileColor = 0;
			float rayAngle = (p.A + FOV / 2.0f) - ((float)x / (float)screenWidth) * FOV;
			if (rayAngle > _2PI) { rayAngle -= _2PI; }
			if (rayAngle < 0) { rayAngle += _2PI; }
			float distanceToWall = 0;
			bool hitCorner = false; // flag for hitting corners of a map block
			float tanR = tanf(rayAngle);
			
			// - - - - horizontal rays - - - -
			float hX, hY, stepX = 0, stepY = 0, disH = ROV;
			bool lookUp = false, hitWallH = false;
			if (rayAngle == 0 || rayAngle == PI) { // looking straight horizontal
				hitWallH = true;
				hY = p.Y;
				hX = p.X;
			} else if (rayAngle < PI) { // looking up
				lookUp = true;
				stepY = 1;
				hY = ceil(p.Y);
				hX = p.X + ((stepY * dy) / tanR);
			} else {
				stepY = -1;
				hY = floor(p.Y);
				hX = p.X + ((stepY * d2y) / tanR);
			}
			stepX = stepY / tanR;
			int mapPosH = 0;
			while (!hitWallH) {
				int mapX = (int)hX;
				int mapY = lookUp ? (int)hY : (int)hY - 1;
				//test if ray is out of bounds
				if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
					disH = ROV;
					break;
				} else {
					mapPosH = mapY * mapWidth + mapX;
					if (map[mapPosH] > 0) {
						hitWallH = true;
						disH = dist(p.X, p.Y, hX, hY, p.A);
					} else {
						hX += stepX;
						hY += stepY;
					}
				}
			}
			// - - - - vertical rays - - - -
			float vX, vY, stepVX = 0, stepVY = 0, disV = ROV;
			bool hitWallV = false, lookLeft = false;
			if (rayAngle == PI / 2 || rayAngle == 3 * PI / 2) { // looking straight vertical
				hitWallV = true;
				vY = p.Y;
				vX = p.X;
			} else if (rayAngle < PI / 2 || rayAngle > 3 * PI / 2) { // looking right
				stepVX = 1;
				vX = ceil(p.X);
				vY = p.Y + ((stepVX * dx) * tanR);
			} else {
				lookLeft = true;
				stepVX = -1;
				vX = floor(p.X);
				vY = p.Y + ((stepVX * d2x) * tanR);
			}
			stepVY = stepVX * tanR;
			int mapPosV = 0;
			while (!hitWallV) {
				int mapX = lookLeft? (int)vX -1 : (int)vX;
				int mapY = (int)vY;
				//test if ray is out of bounds
				if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
					disV = ROV;
					break;
				} else {
					mapPosV = mapY * mapWidth + mapX;
					if (map[mapPosV] > 0) { // hit wall
						disV = dist(p.X, p.Y, vX, vY, p.A);
						hitWallV = true;
					} else { // add offset
						vX += stepVX;
						vY += stepVY;
					}
				}
			}
			int horizontalShade = 0;
			if (disH <= disV) {
				distanceToWall = disH;
				mapPos = mapPosH;
				horizontalShade = 0x0000; //"" - microsoft
				if (abs(hX - trunc(hX)) < 0.03) {hitCorner = true;}
			} else {
				distanceToWall = disV;
				mapPos = mapPosV;
				if (abs(vY - trunc(vY)) < 0.03) {hitCorner = true;}
			}
			//distanceToWall = disH;
			// Calculate distance to ceiling and floor
			int ceiling = float(screenHeight / 2.0 - screenHeight / ((float)distanceToWall));
			int floor = screenHeight - ceiling;

			//Shading using Unicode Characters
			int shade = ' ';
			if (hitCorner) { shade = ' '; }
			else if (distanceToWall <= ROV * 0.25) { shade = 0x2588; } // very close
			else if (distanceToWall < ROV * 0.5) { shade = 0x2593; }
			else if (distanceToWall < ROV * 0.75) { shade = 0x2592; }
			else if (distanceToWall < ROV) { shade = 0x2591; }
			else { shade = ' '; }    //too far away

			for (int y = 0; y < screenHeight; y++) {
				if (y <= ceiling) {
					screen[y * screenWidth + x].Char.AsciiChar = 96;
				} else if (y > ceiling && y <= floor) {
					screen[y * screenWidth + x].Char.AsciiChar = shade;
				} else {
					//Shade floor based on distance
					float floorDist = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
					int floorShade = ' ';
					if (floorDist < 0.25) { floorShade = '#'; }
					else if (floorDist < 0.6) { floorShade = '-'; }//-
					else if (floorDist < 0.8) { floorShade = '.'; }//.
					else/* if (floorDist < 0.9)*/ { floorShade = 'x'; }//x
					//else						{ shade = 32; }
					screen[y * screenWidth + x].Char.AsciiChar = floorShade;
				}
			screen[y * screenWidth + x].Attributes = map[mapPos] | horizontalShade;
			}
			mapPos = 0;
		}
		// - - - - - RENDERING - - - - -
		draw_map(screen, test_map, 16, 16, screenWidth, 3);
		float fps = 1.0f / elapsedTime; //get framerate
		std::wstring title = std::to_wstring(fps);
		SetConsoleTitle(title.data());
		WriteConsoleOutput(console, screen.data(), {short(screenWidth), short(screenHeight)}, {0, 0}, &windowSize);
	}
	return 0;
}

int createConsole(SMALL_RECT &w, HANDLE &h) {
	SetConsoleWindowInfo(h, true, &w);
	COORD coord = { short(screenWidth), short(screenHeight) };
	if (!SetConsoleScreenBufferSize(h, coord)) {
		//"Wrong buffer size"
		return 1;
	}
	if (!SetConsoleActiveScreenBuffer(h)) {
		//"Set Active Screen Buffer error"
		return 2;
	}
	CONSOLE_FONT_INFOEX font_info = { sizeof(font_info), 0, fontWidth, fontHeight, FF_DONTCARE, FW_NORMAL, L"Consolas" };
	if (!SetCurrentConsoleFontEx(h, false, &font_info)) {
		//"Font error"
		return 3;
	}
	CONSOLE_SCREEN_BUFFER_INFO buff_info;
	if (!GetConsoleScreenBufferInfo(h, &buff_info)) {
		// "buffer error"
		return 4;
	}
	if (screenHeight > buff_info.dwMaximumWindowSize.Y) {
		//"Height overload"
		//return buff_info.dwMaximumWindowSize.Y;
		return 5;
	}
	if (screenWidth > buff_info.dwMaximumWindowSize.X) {
		//"Width overload"
		return 6;
	}
	w = { 0, 0, short(screenWidth - 1), short(screenHeight - 1) }; // set actual window size;
	if (!SetConsoleWindowInfo(h, true, &w)) {
		// "window error"
		return 7;
	}
	//if (!SetConsoleMode(consoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))  < - - - mouse input
	return 0;
}

//screen, map to draw, map width, map height, screen width, pixel scale
void draw_map(std::vector<CHAR_INFO> &s, const std::vector<int> &m, size_t mw, size_t mh, size_t sw, size_t ps) {
	for (size_t i = 0; i < mh; i++) {
		for (size_t j = 0; j < mw; j++) {
			int pixelChar = 96;
			if (m[i * mh + j]) { pixelChar = 0x2588; }
			for (size_t k = 0; k < ps; k++) {
				for (size_t l = 0; l < ps; l++) {
					s[(i * ps + k) * sw + j*ps + l].Char.AsciiChar = pixelChar;
				}
			}
			
		}
	}
}

void clearScreen(std::vector<CHAR_INFO>& s) {
	for (size_t i = 0; i < screenSize; i++) {
		s[i].Char.AsciiChar = 0;
		s[i].Attributes = 0x0000;
	}
}

//flips a map from 0 1 2 to 6 7 8 to avoid mirrored map effect
//				   3 4 5	3 4 5
//				   6 7 8	0 1 2
std::vector<int> map_parser(std::vector<int> inMap, int mWidth, int mHeight) {
	std::vector<int> res(inMap.size());
	for (int i = 0; i < mHeight; ++i) {
		for (int j = 0; j < mWidth; ++j) {
			res.at(res.size() - ((i + 1) * mWidth) + j) = inMap.at(i * mWidth + j);
		}
	}
	return res;
}

float dist(float ax, float ay, float bx, float by, float angle) {
	return (bx - ax) * cosf(angle) + (by - ay) * sinf(angle);
}