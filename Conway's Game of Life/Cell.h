#pragma once
class Cell{
	bool alive;

public:

	Cell(): alive(false) {}

	void create() {
		alive = true;
	}

	void kill() {
		alive = false;
	}

	bool status() const {
		return alive;
	}
};

