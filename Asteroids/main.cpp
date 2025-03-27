#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "ECSLib.h"

struct Vector2 {
	Vector2() {
		reset();
	}
	Vector2(float val) {
		x = val;
		y = val;
	}
	Vector2(float _x, float _y) {
		x = _x;
		y = _y;
	}
	static Vector2 lerp(Vector2 a, Vector2 b, float t) {
		return a * (1 - t) + b * t;
	}
	static Vector2 Unit(float angle) {
		return Vector2(cos(angle), sin(angle));

	}
	float x, y;

	void reset() {
		x = 0;
		y = 0;
	}
	float mag() {
		return sqrtf(powf(x, 2) + powf(y, 2));
	}
	float angle() {
		return atan2f(y, x);
	}
	void operator=(float val) {
		x = val;
		y = val;
	}
	Vector2 operator+(Vector2 vec) {
		Vector2 sum = { x + vec.x, y + vec.y };
		return sum;
	}
	Vector2 operator-(Vector2 vec) {
		Vector2 dif = { x - vec.x, y - vec.y };
		return dif;
	}
	Vector2 operator*(float factor) {
		Vector2 product = { x * factor, y * factor };
		return product;
	}
	Vector2 operator*(Vector2 vector) {
		Vector2 product = { x * vector.x, y * vector.y };
		return product;
	}
	Vector2 operator/(float factor) {
		Vector2 product = { x / factor, y / factor };
		return product;
	}
	void operator+=(Vector2 vec) {
		x += vec.x;
		y += vec.y;
	}
	void operator-=(Vector2 vec) {
		x -= vec.x;
		y -= vec.y;
	}
	void operator*=(float divisor) {
		x *= divisor;
		y *= divisor;
	}
	void operator/=(float divisor) {
		x /= divisor;
		y /= divisor;
	}
	bool operator==(Vector2 comparator) {
		return (x == comparator.x && y == comparator.y);
	}
};

struct Sprite {
	SDL_Texture* texture;
	SDL_Rect clip;
};

struct SDLton : Singleton {
	bool SDLInit()
	{
		//Initialization flag
		bool success = true;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create window
			gWindow = SDL_CreateWindow("Asteroids", SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_RESIZABLE);
			if (gWindow == NULL)
			{
				printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Create vsynced renderer for window
				gRenderer = SDL_CreateRenderer(gWindow, NULL);
				if (gRenderer == NULL)
				{
					printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
					success = false;
				}
				else
				{
					//Initialize renderer color
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				}
			}
		}
		SDL_GetKeyboardState(keyboard);

		return success;
	}

	void SDLClose()
	{
		//Destroy window	
		SDL_DestroyRenderer(gRenderer);
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;
		gRenderer = NULL;
		SDL_Quit();
	}

	//Screen dimension constants
	const int SCREEN_WIDTH = 1920;
	const int SCREEN_HEIGHT = 1080;

	//Keyboard
	int* keyboard;

	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;

	//The window renderer
	SDL_Renderer* gRenderer = NULL;

	SDL_Color bground = { 110,58,80 };

	//SDLImage
	std::unordered_map<std::string, SDL_Texture*> textures;
	std::unordered_map<std::string, Sprite> sprites;


	void CreateSprite(std::string name, std::string path, SDL_Rect clip) {
		if (textures.find(path) == textures.end()) {
			LoadTexture(path);
		}

		sprites[name] = Sprite{ textures[path], clip };
	}
	void LoadTexture(std::string path) {
		if (textures[path] == NULL) {
			//The final texture
			SDL_Texture* newTexture = NULL;

			//Load image at specified path
			SDL_Surface* loadedSurface = IMG_Load(path.c_str());
			if (loadedSurface == NULL)
			{
				printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError());
			}
			else
			{
				//Create texture from surface pixels
				newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
				if (newTexture == NULL)
				{
					printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
				}

				//Get rid of old loaded surface
				SDL_DestroySurface(loadedSurface);
			}

			textures[path] = newTexture;
		}
	}

	Sprite GetSprite(std::string name) {
		return sprites[name];
	}
};

struct Transform {
	Vector2 pos;
};

struct SpriteRenderer {
	Vector2 pos;
	std::string sprite;
};

class RenderSys : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();
		
		SDL_SetRenderDrawColor(sdl.gRenderer, sdl.bground.r, sdl.bground.g, sdl.bground.b, 255);
		SDL_RenderClear(sdl.gRenderer);

		for (auto object : ObjectsWith<SpriteRenderer>()) {
			auto& spriteRenderer = object.GetComponent<SpriteRenderer>();
			auto& xform = object.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			const SDL_FRect rect = {(float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			const SDL_FRect destRect = { rect.x + xform.pos.x,rect.y + xform.pos.y,rect.w*10,rect.h*10 };
			const SDL_FPoint center = { 0,0 };
			SDL_RenderTextureRotated(sdl.gRenderer, sprite.texture, &rect, &destRect,0, &center,SDL_FLIP_NONE);

		}

		SDL_RenderPresent(sdl.gRenderer);
	}
};

class AsteroidsScene : public Scene {
	void Init() {
		RegisterComponents<Transform, SpriteRenderer>();
		RegisterSystems<RenderSys>();

		DefineObject<Transform,SpriteRenderer>("ship");

		Object o = CreateObject("ship");
		o.GetComponent<Transform>().pos = { 200,200 };
		o.GetComponent<SpriteRenderer>().pos = { 0,0 };
		o.GetComponent<SpriteRenderer>().sprite = "ship";
	}
	void Quit() {
		
	}
};

class AsteroidsGame : public Game {
protected:
	void Init() {
		// Create a Singleton for rendering data, accessible across Scenes
		CreatePersistentSingletons<SDLton>();
		auto& sdl = GetPersistentSingleton<SDLton>();
		// Access and initialize the renderer
		sdl.SDLInit();
		sdl.CreateSprite("ship", "Assets/ship.png", { 0,0,48,48 });

		//Register Scenes
		RegisterScene<AsteroidsScene>("AsteroidsScene");
	}

	void Quit() {
		GetPersistentSingleton<SDLton>().SDLClose();
	}
};

int main() {
	AsteroidsGame game;
	game.Start("AsteroidsScene");

	return 0;
}