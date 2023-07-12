#pragma once
#include "Cell.h"
#include <chrono>
#include <vector>

class Grid{
public:
	const int width;
	const int height;
	std::vector<Cell> cells;

	Grid(int n, int width, int height) :cells(), width(width), height(height) {
		cells.resize(n);
	}

	void populate_grid();

	bool will_survive(int x, int y);

	bool will_be_created(int x, int y);

	void update(const Grid& next);
};

void calculate(Grid& old_generation, Grid& new_generation);