#include <chrono>
#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>


float dist(float ax, float ay, float bx, float by, float ra);
std::vector<int> map_parser(std::vector<int> inMap, int mWidth, int mHeight);
std::vector<int> randomize_map(int width, int height, int cw = 1);
//bool collision_check(float pa);

constexpr auto PI = 3.1415926f;
constexpr float _2PI = 2 * PI;
constexpr float D360 = 2 * PI;
constexpr float D270 = 3 * PI / 2;
constexpr float D180 = PI;
constexpr float D90 = PI / 2;

const int screenWidth = 800;
const int screenHeight = 600;
const int sourceMapWidth = 8;
const int sourceMapHeight = 8;
const int corridorWidth = 3;
const int mapHeight = sourceMapHeight * (corridorWidth + 1) + 1;
const int mapWidth = sourceMapWidth * (corridorWidth + 1) + 1;

class Player {
public:
	float X = 1.5f; 
	float Y = 1.5f;
	float A = 0; //angle player is looking at - 0 means right; in radians;
};

float FOV = PI / 3; // field of view
float ROV = 16.0f; //range of view
int speed = 10;

int main(){
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "FPS_01");

	std::vector<int> input{ {
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
		1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,
		1,0,1,0,1,0,0,0,1,0,0,0,0,1,1,1,
		1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,1,
		1,0,1,0,1,0,0,0,0,0,0,0,1,0,0,1,
		1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	} };

	std::vector<int> map = map_parser(randomize_map(sourceMapWidth, sourceMapHeight, corridorWidth), mapWidth, mapHeight);

	Player p{};

	std::vector<sf::Vertex> columns(screenWidth * 2);

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

    while (window.isOpen()) {

        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
		// - - - - - TIMING - - - - - -
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed = tp2 - tp1;
		tp1 = tp2;
		float elapsedTime = elapsed.count();

		// - - - - - CONTROLS - - - - -
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			p.A += 1.3f * elapsedTime;
			if (p.A > _2PI) { p.A -= _2PI; }
		} 
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			p.A -= 1.3f * elapsedTime;
			if (p.A < 0) { p.A += _2PI; }
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			p.X += cosf(p.A) * elapsedTime * 5.0f;
			p.Y += sinf(p.A) * elapsedTime * 5.0f;
			//collision detection
			if (map[int(p.Y) * mapWidth + int(p.X)] > 0) {
				p.X -= cosf(p.A) * elapsedTime * 5.0f;
				p.Y -= sinf(p.A) * elapsedTime * 5.0f;
			}
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			p.X -= cosf(p.A) * elapsedTime * 5.0f;
			p.Y -= sinf(p.A) * elapsedTime * 5.0f;
			//collision detection
			if (map[int(p.Y) * mapWidth + int(p.X)] > 0) {
				p.X += cosf(p.A) * elapsedTime * 5.0f;
				p.Y += sinf(p.A) * elapsedTime * 5.0f;
			}
		}


		window.clear();
		//- - - - - - RAYCASTING - - - - - -
		float dx = ceilf(p.X) - p.X; // X offset within tile
		float d2x = p.X - floorf(p.X);
		float dy = ceilf(p.Y) - p.Y; // Y offset within tile
		float d2y = p.Y - floorf(p.Y);
		int mapPos = 0;
		for (int x = 0; x < screenWidth; x++) {
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
			//int horizontalShade = 0;
			if (disH <= disV) {
				distanceToWall = disH;
				mapPos = mapPosH;
				if (abs(hX - trunc(hX)) < 0.03) {hitCorner = true;}
			} else {
				distanceToWall = disV;
				mapPos = mapPosV;
				if (abs(vY - trunc(vY)) < 0.03) {hitCorner = true;}
			}
			
			//add ready colors
			uint8_t shade = 255;

			//if (distanceToWall <= ROV * 0.25) { shade = 0; } // very close
			if (distanceToWall < ROV * 0.5) { shade = 191; }
			else if (distanceToWall < ROV * 0.75) { shade = 128; }
			else if (distanceToWall < ROV) { shade  = 64; }
			else { shade = 16; }
			sf::Color columnColor = hitCorner? sf::Color(0,0,0) : sf::Color(255, 128, 0, shade);
			//draw 3D walls
			int h1 = float(screenHeight / 2.0 - screenHeight / ((float)distanceToWall));
			int h2 = screenHeight - h1;
			columns[x*2] = sf::Vertex(sf::Vector2f( x, h1), columnColor);
			columns[x*2+1] = sf::Vertex(sf::Vector2f(x, h2), columnColor);
		}

		window.draw(columns.data(), screenWidth * 2, sf::Lines);
		float fps = 1.0f / elapsedTime;
		window.setTitle(std::to_string(fps));

        window.display();
    }
    return 0;
}
//flips a map from 0 1 2 to 6 7 8 to avoid mirrored map effect
//				   3 4 5	3 4 5
//				   6 7 8	0 1 2
std::vector<int> map_parser(std::vector<int> inMap, int mWidth, int mHeight) {
	std::vector<int> res(inMap.size());
	for (int i{ 0 }; i < mHeight; ++i) {
		for (int j{ 0 }; j < mWidth; ++j) {
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
				res_map[i * rw + j] = 3;
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