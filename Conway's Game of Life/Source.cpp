#include <chrono>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

//#include "Cell.h"
#include "Grid.h"

const int screenWidth = 1280;
const int screenHeight = 960;
const int screenSize = screenHeight * screenWidth;
const int pixelRatio = 4; //pixel ratio
const int gridWidth = screenWidth / pixelRatio;
const int gridHeight = screenHeight / pixelRatio;
const int gridSize = gridHeight * gridWidth;

int brushSize = 1;

void prepare_grid(sf::VertexArray& grid);
void set_grid_color(sf::VertexArray& display_grid, Grid& info_grid);
void user_paints_grid(sf::Vector2i& mouse_pos, Grid& grid, int brushSize, bool kill);
void brush2(sf::Vector2i& pos, Grid& grid, bool kill);
void brush3(sf::Vector2i& pos, Grid& grid, bool kill);

int main() {
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Conway's Game of Life");
    sf::Time interval = sf::milliseconds(16);
    sf::Clock clock;
    sf::Clock textClock;
    bool isTime = false;
    bool displayText = false;

    sf::Font font;
    if (!font.loadFromFile("dina10px.ttf")) { return 0xFF; };

    sf::Text text("", font, 48);
    text.setFillColor(sf::Color::White);
    text.setPosition(20.f, 20.f);

    Grid old_grid(gridSize, gridWidth, gridHeight);
    Grid new_grid(gridSize, gridWidth, gridHeight);
    new_grid.populate_grid();

    sf::VertexArray image(sf::Triangles, gridSize * 6);
    prepare_grid(image);

    while (window.isOpen()) {
        // - - - - - - - TIMING - - - - - - -
        sf::Time elapsed = clock.getElapsedTime();
        isTime = elapsed >= interval;
        
        sf::Time elapsedText = textClock.getElapsedTime();
        displayText = elapsedText <= sf::milliseconds(800);

        if (isTime && interval < sf::milliseconds(2000)) {
            calculate(old_grid, new_grid);    //populate next generation
            clock.restart();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            //get user mouse input
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2i localPosition = sf::Mouse::getPosition(window);
                user_paints_grid(localPosition, new_grid, brushSize, false);
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                sf::Vector2i localPosition = sf::Mouse::getPosition(window);
                user_paints_grid(localPosition, new_grid, brushSize, true);
            }
            //get user keyboard input
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                brushSize = 1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                brushSize = 2;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                brushSize = 3;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                new_grid.populate_grid();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add)) {
                interval -= (interval > sf::milliseconds(1000))? sf::milliseconds(250) : sf::microseconds(5000);
                if (interval < sf::microseconds(1)) { interval = sf::microseconds(1); }
                text.setString(std::to_string(interval.asMilliseconds()) + "ms");
                textClock.restart();

            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)) {
                interval += (interval > sf::milliseconds(1000))? sf::milliseconds(250) : sf::microseconds(5000);
                if (interval > sf::milliseconds(2000)) { interval = sf::milliseconds(2000); }
                text.setString(std::to_string(interval.asMilliseconds()) + "ms");
                textClock.restart();
            }
        }

        set_grid_color(image, new_grid);

        old_grid.update(new_grid);        //update to next generation
        
        window.clear(sf::Color::Black);
        window.draw(&image[0], gridSize *6, sf::Triangles);
        // - - - - - MESSAGE - - - - -
        if (displayText) window.draw(text);
        window.display();
    }
    return 0;
}

void prepare_grid(sf::VertexArray &grid) {
    int i = 0;
    for (int y = 0; y < screenHeight; y += pixelRatio) {
        for (int x = 0; x < screenWidth; x += pixelRatio) {
            grid[i    ].position = {static_cast<float>(x),              static_cast<float>(y)};
            grid[i + 1].position = {static_cast<float>(x + pixelRatio), static_cast<float>(y)};
            grid[i + 2].position = {static_cast<float>(x),              static_cast<float>(y + pixelRatio)};
            grid[i + 3].position = {static_cast<float>(x),              static_cast<float>(y + pixelRatio)};
            grid[i + 4].position = {static_cast<float>(x + pixelRatio), static_cast<float>(y)};
            grid[i + 5].position = {static_cast<float>(x + pixelRatio), static_cast<float>(y + pixelRatio)};
            i += 6;
        }
    }
}

void set_grid_color(sf::VertexArray &display_grid, Grid& info_grid) {
    uint8_t opaqueness = 0;
    for (int i = 0; i < gridSize; i++) {
        if (info_grid.cells[i].status())
            opaqueness = 255;
        else
            opaqueness = 0;
        display_grid[6 * i    ].color = sf::Color(255, 128, 0, opaqueness);
        display_grid[6 * i + 1].color = sf::Color(255, 128, 0, opaqueness);
        display_grid[6 * i + 2].color = sf::Color(255, 128, 0, opaqueness);
        display_grid[6 * i + 3].color = sf::Color(255, 128, 0, opaqueness);
        display_grid[6 * i + 4].color = sf::Color(255, 128, 0, opaqueness);
        display_grid[6 * i + 5].color = sf::Color(255, 128, 0, opaqueness);
    }
}

void user_paints_grid(sf::Vector2i& mouse_pos, Grid& grid, int brushSize, bool kill) {
    mouse_pos.x /= pixelRatio;
    mouse_pos.y /= pixelRatio;
    switch (brushSize) {
    case 3:
        brush3(mouse_pos, grid, kill);
        break;
    case 2:
        brush2(mouse_pos, grid, kill);
        break;
    default:
        unsigned cellPosition = mouse_pos.x + mouse_pos.y * gridWidth;
        if (cellPosition >= gridSize)
            cellPosition = gridSize - 1;
        if (kill)
            grid.cells[cellPosition].kill();
        else
            grid.cells[cellPosition].create();
    }
}

void brush2(sf::Vector2i &pos, Grid& grid, bool kill) {
     sf::Vector2i tiles[] = {{pos.x, pos.y - 1},
         {pos.x - 1, pos.y}, {pos.x, pos.y    }, {pos.x + 1, pos.y},
                             {pos.x, pos.y + 1}};
     for (int i = 0; i < 5; i++) {
         if (tiles[i].x > 0 && tiles[i].x < gridWidth - 1 && tiles[i].y > 0 && tiles[i].y < gridHeight - 1) {
             if (kill)
                 grid.cells[tiles[i].x + tiles[i].y * gridWidth].kill();
             else
                 grid.cells[tiles[i].x + tiles[i].y * gridWidth].create();
         }
     }
}

void brush3(sf::Vector2i& pos, Grid& grid, bool kill) {
    /*
    - - X - - <-- brush on grid example
    - X X X -
    X X X X X
    - X X X -
    - - X - -
    */
    sf::Vector2i tiles[] = {
                                                    {pos.x, pos.y - 2},
                            {pos.x - 1, pos.y - 1}, {pos.x, pos.y - 1}, {pos.x + 1, pos.y - 1}, 
        {pos.x - 2, pos.y}, {pos.x - 1, pos.y    }, {pos.x, pos.y    }, {pos.x + 1, pos.y    }, {pos.x + 2, pos.y},
                            {pos.x - 1, pos.y + 1}, {pos.x, pos.y + 1}, {pos.x + 1, pos.y + 1},
                                                    {pos.x, pos.y + 2}
    };

     for (int i = 0; i < 13; i++) {
         if (tiles[i].x > 0 && tiles[i].x < gridWidth - 1 && tiles[i].y > 0 && tiles[i].y < gridHeight - 1) {
             if (kill)
                 grid.cells[tiles[i].x + tiles[i].y * gridWidth].kill();
             else
                 grid.cells[tiles[i].x + tiles[i].y * gridWidth].create();
         }
     }
}