#pragma once
#include "Vector2.h"
#include <string>
#include <SDL3/SDL_render.h>

// Transform: component for storing the positional data of an object.
struct Transform {
	Vector2 position = { 0,0 };
	Vector2 velocity = { 0,0 };
	float angularVelocity = 0;
	float rotation = 0;
};

// SpriteRenderer: component for storing the name of a visible object's sprite.
struct SpriteRenderer {
	std::string sprite = "";
};

// TextRenderer: component for storing a text message to be displayed.
struct TextRenderer {
	std::string message = "";
	std::string currentMessage = "";
	SDL_Texture* texture = nullptr;
	bool visible = true;
};

// DestroyTimer: component for storing the countdown timer for an object's destruction.
struct DestroyTimer {
	int countdown = 20;
};

// Asteroid: component for storing the size of an asteroid.
struct Asteroid {
	int size = 0;
};

// Score: component for storing the in-game score.
struct Score {
	int score = 0;
};

// InstructionsTimer: component for storing the duration of instructions.
struct InstructionsTimer {
	int timer = 0;
};

// Ship: component for storing the reload countdown timer and the state of the spacebar.
struct Ship {
	int reloadTimer = 0;
	bool spaceHeld = false;
};