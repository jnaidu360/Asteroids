#include "Singletons.h"

bool SDLton::SDLInit()
{
	// Define a flag for successful initialization
	bool success = true;

	// Try to initialize the SDL library for input and rendering
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		// Create and initialize an application window and renderer
		window = SDL_CreateWindow("Asteroids", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, NULL);

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		renderTexture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET,
			SCREEN_WIDTH, SCREEN_HEIGHT
		);
		// Try to intitialize the SDL_ttf library for text rendering
		if (TTF_Init() == -1) {
			fprintf(stderr, "TTF_Init Error: %s\n", SDL_GetError());
			SDL_Quit();
			return 1;
		}
		else {
			// Open the retro font asset
			font = TTF_OpenFont("Assets/Retro.ttf", 48);
		}
	}
	// Get a pointer to SDL's keyboard input
	keyboard = SDL_GetKeyboardState(NULL);
	// Set the vsync to display current state with every monitor refresh
	SDL_SetRenderVSync(renderer, 1);

	// Load sprite assets used by the game
	CreateSprite("ship", "Assets/ship-a1.png", { 0,0,48,48 });
	CreateSprite("ship_accel", "Assets/ship-a2.png", { 0,0,48,48 });
	CreateSprite("ship_reverse", "Assets/ship-a3.png", { 0,0,48,48 });
	CreateSprite("ship_reloading", "Assets/ship-a4.png", { 0,0,48,48 });
	CreateSprite("ship_accel_reloading", "Assets/ship-a5.png", { 0,0,48,48 });
	CreateSprite("ship_reverse_reloading", "Assets/ship-a6.png", { 0,0,48,48 });
	CreateSprite("big", "Assets/big-a.png", { 0,0,48,48 });
	CreateSprite("med", "Assets/med-a.png", { 0,0,48,48 });
	CreateSprite("small", "Assets/small-a.png", { 0,0,48,48 });
	CreateSprite("bullet", "Assets/bullet-b1.png", { 0,0,16,16 });
	CreateSprite("explosion", "Assets/explosions-a5.png", { 0,0,32,32 });

	// Return the success flag
	return success;
}

// Close and free all loaded memory
void SDLton::SDLClose()
{
	for (auto tex : textures) {
		SDL_DestroyTexture(tex.second);
	}
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;
	SDL_Quit();
}

// Function to create texture from path and assign it a name
void SDLton::CreateSprite(std::string name, std::string path, SDL_Rect clip) {
	if (textures.find(path) == textures.end()) {
		LoadTexture(path);
	}

	sprites[name] = Sprite{ textures[path], clip };
}

// Helper function to load texture from path
void SDLton::LoadTexture(std::string path) {
	if (textures[path] == NULL) {
		SDL_Texture* newTexture = NULL;

		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
		if (loadedSurface == NULL)
		{
			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			if (newTexture == NULL)
			{
				printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			}

			SDL_DestroySurface(loadedSurface);
		}

		textures[path] = newTexture;
		SDL_SetTextureScaleMode(textures[path], SDL_SCALEMODE_NEAREST);
	}
}

// Function to load the given message using the retro font
SDL_Texture* SDLton::LoadText(std::string textContent) {
	SDL_Surface* text = TTF_RenderText_Blended(font, textContent.c_str(), textContent.size(), SDL_Color{ 255,255,255,255 });
	SDL_Texture* texture = NULL;
	if (text) {
		texture = SDL_CreateTextureFromSurface(renderer, text);
		SDL_DestroySurface(text);
	}
	if (!texture) {
		SDL_Log("Couldn't create text: %s\n", SDL_GetError());
	}
	else {
		return texture;
	}
	return nullptr;
}

// Helper function to get a sprite by name
Sprite SDLton::GetSprite(std::string name) {
	return sprites[name];
}
