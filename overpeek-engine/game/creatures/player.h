#pragma once

#include "../logic/game.h"
#include "creature.h"

class Player : public Creature {
public:
	Inventory *inventory;

public:
	Player() {}
	Player(float x, float y, Inventory *inv);

	void submitToRenderer(graphics::Renderer *renderer, float renderOffsetX, float renderOffsetY);
	void update();
	void hit();
	void place();
};