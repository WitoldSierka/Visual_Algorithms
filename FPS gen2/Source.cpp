#include <algorithm>
#include <chrono>
#include <math.h>
#include <stack>
#include <string>
#include <vector>
#include <Windows.h>

constexpr float PI = 3.14159262f;
constexpr float _2PI = 2 * PI;
constexpr float D360 = 2 * PI;
constexpr float D270 = 3 * PI / 2;
constexpr float D180 = PI;
constexpr float D90 = PI / 2;

int screenWidth = 400;
int screenHeight = 200;
size_t screenSize = screenWidth * screenHeight;
short fontWidth = 3;
short fontHeight = 3;
int sourceMapHeight = 8;
int sourceMapWidth = 8;
int corridorWidth = 3;
int mapHeight = sourceMapHeight * (corridorWidth + 1) + 1;
int mapWidth = sourceMapWidth * (corridorWidth + 1) + 1;
int mapColor = 2;
float FOV = PI / 3;

class Player {
public:
	float A = 0;
	float X = 1.5;
	float Y = 1.5;
};

enum PIXEL_RANGE {
	PX100 = 0x2588,
	PX75 = 0x2593,
	PX50 = 0x2592,
	PX25 = 0x2591,
	PX00 = 160  //non-breaking space
};

void clearScreen(std::vector<CHAR_INFO>& s);
int createConsole(SMALL_RECT& w, HANDLE& h);
float dist(float ax, float ay, float bx, float by, float angle);
void draw_map(std::vector<CHAR_INFO>& s, const std::vector<int>& m, size_t mw, size_t mh, size_t sw, const Player &p, size_t ps = 1);
std::vector<int> map_parser(std::vector<int> inMap, int mWidth, int mHeight);
std::vector<int> randomize_map(int width, int height, int cw = 1);

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
// 0 - black
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

std::vector<int> map = map_parser(randomize_map(sourceMapWidth, sourceMapHeight, corridorWidth), mapWidth, mapHeight);

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
	bool mapToggle = true;
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	Player p;
	float ROV = 16.0f; //range of view

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
		if (GetAsyncKeyState((unsigned short)'M') & 0x8000) {
			mapToggle = mapToggle ? false : true;
		}
		// - - - - - RAYCASTING - - - - -

		float dx = ceilf(p.X) - p.X; // X offset within tile
		float d2x = p.X - floorf(p.X);
		float dy = ceilf(p.Y) - p.Y; // Y offset within tile
		float d2y = p.Y - floorf(p.Y);
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
				hY = ceilf(p.Y);
				hX = p.X + ((stepY * dy) / tanR);
			} else {
				stepY = -1;
				hY = floorf(p.Y);
				hX = p.X + ((stepY * d2y) / tanR);
			}
			stepX = stepY / tanR;
			int mapPosH = 0;
			while (!hitWallH) {
				int mapX = (int)hX;
				int mapY = lookUp ? (int)hY : (int)hY - 1;
				//test if ray is out of bounds
				if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
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
				vX = ceilf(p.X);
				vY = p.Y + ((stepVX * dx) * tanR);
			} else {
				lookLeft = true;
				stepVX = -1;
				vX = floorf(p.X);
				vY = p.Y + ((stepVX * d2x) * tanR);
			}
			stepVY = stepVX * tanR;
			int mapPosV = 0;
			while (!hitWallV) {
				int mapX = lookLeft? (int)vX -1 : (int)vX;
				int mapY = (int)vY;
				//test if ray is out of bounds
				if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
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
			//int horizontalShade = 0;
			if (disH <= disV) {
				distanceToWall = disH;
				mapPos = mapPosH;
				//horizontalShade = 0x80;
				if (abs(hX - trunc(hX)) < 0.015 || abs(ceil(hX) - hX) < 0.015) {hitCorner = true;}
			} else {
				distanceToWall = disV;
				mapPos = mapPosV;
				if (abs(vY - trunc(vY)) < 0.03) {hitCorner = true;}
			}
			
			// Calculate ceiling and floor thresholds
			int ceiling = static_cast<int>(screenHeight / 2.0f - screenHeight / (float)distanceToWall);
			int floor = screenHeight - ceiling;

			//Shading using Unicode Characters
			int shade = ' ';
			if (hitCorner) { shade = ' '; }
			else if (distanceToWall < ROV * 0.25)	{ shade = PX100;}
			else if (distanceToWall < ROV * 0.5)	{ shade = PX75; }
			else if (distanceToWall < ROV * 0.75)	{ shade = PX50; }
			else if (distanceToWall < ROV)			{ shade = PX25; }
			else									{ shade = PX00; }

			for (int y = 0; y < screenHeight; y++) {
				screen[y * screenWidth + x].Attributes = map[mapPos] ? map[mapPos] : mapColor;
				if (y <= ceiling) {
					screen[y * screenWidth + x].Char.UnicodeChar = '`';
				} else if (y > ceiling && y <= floor) {
					screen[y * screenWidth + x].Char.UnicodeChar = shade;
					//screen[y * screenWidth + x].Attributes |= horizontalShade;
				} else {
					//Shade floor based on distance
					float floorDist = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
					int floorShade = ' ';
					if (floorDist < 0.25)	  { floorShade = 'x'; }
					else if (floorDist < 0.6) { floorShade = '+'; }
					else if (floorDist < 0.8) { floorShade = '-'; }
					else					  { floorShade = '.'; }
					screen[y * screenWidth + x].Char.UnicodeChar = floorShade;
				}
			}
			mapPos = 0;
		}
		// - - - - - RENDERING - - - - -
		if (mapToggle) {
			draw_map(screen, map, mapWidth, mapHeight, screenWidth, p, 3);
		}
		float fps = 1.0f / elapsedTime;
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
	return 0;
}

//screen, map to draw, map width, map height, screen width, player position, pixel scale
void draw_map(std::vector<CHAR_INFO> &s, const std::vector<int> &m, size_t mw, size_t mh, size_t sw, const Player &p, size_t ps) {
	for (int i = 0; i < mh; i++) {
		for (int j = 0; j < mw; j++) {
			int pixelChar = PX00;
			if (m[i * mh + j]) { pixelChar = PX100; }
			for (size_t k = 0; k < ps; k++) {
				for (size_t l = 0; l < ps; l++) {
					s[(i * ps + k) * sw + j*ps + l].Char.UnicodeChar = pixelChar;
				}
			}
		}
	}
	for (size_t i = 0; i < ps; i++) {
		for (size_t j = 0; j < ps; j++) {
			size_t pos = (int(p.Y) * ps + i) * sw + int(p.X) * ps + j;
			s[pos].Attributes = 4;
			s[pos].Char.UnicodeChar = PX100;
		}
	}
}

void clearScreen(std::vector<CHAR_INFO>& s) {
	for (size_t i = 0; i < screenSize; i++) {
		s[i].Char.AsciiChar = 0;
		s[i].Attributes = 0;
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

//args: width, height, corridor width
std::vector<int> randomize_map(int width, int height, int cw) {
	cw++;
	int visited = 1, cell_count = width * height, rw = width * cw + 1, rh = height * cw + 1;
	std::vector<int> res_map(rw * rh);
	//create grid
	for (int i = 0; i < rh; ++i) {
		for (int j = 0; j < rw; j++) {
			if (i%cw == 0 || j%cw == 0) {
				res_map[i * rw + j] = mapColor;
			}
		}
	}
	std::vector<int> check_map(cell_count);
	std::stack<std::pair<int, int>> cell_stack;
	cell_stack.push({0,0});
	check_map[0] = 1;
	srand(unsigned(time(NULL)));
	while (visited < cell_count) {
		std::pair<int, int> current = cell_stack.top();
		//check for neighbors
		std::vector<std::pair<int, int>> pos_neighbors;
		std::vector<std::pair<int, int>> neighbors = { {current.first, current.second-1}, {current.first+1, current.second},
														   {current.first, current.second+1}, {current.first-1, current.second}};
		for (size_t i = 0; i < neighbors.size(); i++) {
			if (neighbors[i].first >= 0 &&
				  neighbors[i].first < width &&
				  neighbors[i].second >= 0 &&
				  neighbors[i].second < height &&
				  check_map[neighbors[i].second * width + neighbors[i].first] < 1
				) {
				pos_neighbors.emplace_back(neighbors[i]);
			}
		}
		//if no neighbors availible - go back recursively - pop from stack
		if (pos_neighbors.size() < 1) {
			cell_stack.pop();
			continue;
		}
		//choose neighbor randomly
		int dir = rand() % pos_neighbors.size();
		std::pair<int, int> chosen = pos_neighbors[dir];
		//mark passage on result map
		std::pair<int, int> pass = { 0,0 };
		if (current.first == chosen.first) { // moved on Y axis
			int ay = current.second * cw + 1, by = chosen.second * cw + 1;
			pass.first = chosen.first * cw + 1;
			pass.second = (ay < by) ? by - 1 : ay - 1;
			for (int i = 0; i < cw-1; i++) {
				res_map[pass.second * rw + pass.first] = 0;
				pass.first++;
			}
		} else { // moved on X axis
			int ax = current.first * cw + 1, bx = chosen.first * cw + 1;
			pass.first = (ax < bx) ? bx - 1 : ax - 1;
			pass.second = chosen.second * cw + 1;
			for (int i = 0; i < cw-1; i++) {
				res_map[pass.second * rw + pass.first] = 0;
				pass.second++;
			}
		}
		check_map[chosen.second * width + chosen.first] = 1;
		//go to neighbor - push on stack
		cell_stack.push(chosen);
		//increment visited count
		visited++;
	}
	return res_map;
}