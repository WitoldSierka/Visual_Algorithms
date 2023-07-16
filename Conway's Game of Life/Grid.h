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

	void clear();

	void populate_grid();

	bool will_survive(int x, int y) const;

	bool will_be_created(int x, int y) const;

	void update(const Grid& next);
};

void calculate(const Grid& old_generation, Grid& new_generation);