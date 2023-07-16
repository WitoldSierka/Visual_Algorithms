#include "Grid.h"

void Grid::clear() {
	int size = this->width * this->height;
	for (int i = 0; i < size; i++) {
		this->cells[i].kill();
	}
}

void Grid::populate_grid() {
	srand(unsigned(time(NULL)));
	int size = this->cells.size();
	for (int i = 0; i < size; i++) {
		int chance = rand()%5 + 1;
		if (chance == 1)
			cells[i].create();
		else
			cells[i].kill(); //kill may be redundant at start, but it allows to randomize grid at any moment of runtime.
	}
}

bool Grid::will_survive(int x, int y) const {
	if (x == 0 || x >= width - 1 || y == 0 || y >= height -1)
		return false;
	int neighbor_score = 0;
	// X X X [x-1, y-1], [x, y-1], [x+1, y-1]  00, 01, 02 <- example grid
	// X 0 X [x-1, y],   [self],   [x+1, y]    10, 11, 12
	// X X X [x-1, y+1], [x, y+1], [x+1, y+1]  20, 21, 22
	int neighbors[8] = {{x-1 + width * (y-1)}, { x  + width * (y-1)}, {x+1 + width * (y-1)},
						{x-1 + width * ( y )},						  {x+1 + width * ( y )}, 
						{x-1 + width * (y+1)}, { x  + width * (y+1)}, {x+1 + width * (y+1)}};
	for (int i = 0; i < 8; i++) {
		if (cells.at(neighbors[i]).status())
			neighbor_score++;
	}
	if (neighbor_score == 2 || neighbor_score == 3)
		return true;
	return false;
}

bool Grid::will_be_created(int x, int y) const {
	if (x == 0 || x >= width - 1 || y == 0 || y >= height - 1)
		return false;
	int neighbor_score = 0;
	unsigned neighbors[8] = {{x-1 + width * (y-1)}, { x  + width * (y-1)}, {x+1 + width * (y-1)},
						{x-1 + width * ( y )}, /*  {this cell}	*/	  {x+1 + width * ( y )},
						{x-1 + width * (y+1)}, { x  + width * (y+1)}, {x+1 + width * (y+1)}};
	for (int i = 0; i < 8; i++) { //iterate over neighbours
		if (cells.at(neighbors[i]).status())
			neighbor_score++;
	}
	if (neighbor_score == 3)
		return true;
	return false;
}

void Grid::update(const Grid& next) {
	int limit = this->width * this->height;
	for (int i = 0; i < limit; i++) {
		this->cells[i] = next.cells[i];
	}
}

void calculate(const Grid& old_generation, Grid& new_generation) {
	for (int y = 0; y < old_generation.height; y++) {
		for (int x = 0; x < old_generation.width; x++) {
			if (old_generation.cells.at(x + old_generation.width * y).status()) {
				if (old_generation.will_survive(x, y))
					new_generation.cells.at(x + new_generation.width * y).create();
				else
					new_generation.cells.at(x + new_generation.width * y).kill();
			} else if (old_generation.will_be_created(x, y))
				new_generation.cells.at(x + new_generation.width * y).create();
		}
	}
}
