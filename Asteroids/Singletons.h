#pragma once
#include "SDL3_ttf/SDL_ttf.h"
#include "SDL3_image/SDL_image.h"
#include <SDL3/SDL.h>
#include "ECSLib.h"
#include <unordered_map>
#include <string>

// Supporting class for SDLton
struct Sprite {
	SDL_Texture* texture;
	SDL_Rect clip;
};

struct SDLton : Singleton {
	bool SDLInit();	//Initialize the SDL library
	void SDLClose();	//Close the SDL library

	const int SCREEN_WIDTH = 1920;
	const int SCREEN_HEIGHT = 1080;

	const bool* keyboard;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* renderTexture;
	SDL_Color bground = { 10,18,40 };
	TTF_Font* font;

	std::unordered_map<std::string, SDL_Texture*> textures;
	std::unordered_map<std::string, Sprite> sprites;

	void CreateSprite(std::string name, std::string path, SDL_Rect clip);
	void LoadTexture(std::string path);
	SDL_Texture* LoadText(std::string textContent);
	Sprite GetSprite(std::string name);
};

// Singleton used to store data for asteroid generation rate (and "phases")
struct AsteroidGeneration : Singleton {
	int nextAsteroidCounter = 0;
	int nextAsteroidAt = 180;
	int nextStageCounter = 0;
	int nextStageAt = 20;
	int stageNum = 1;
};