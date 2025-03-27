#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "ECSLib.h"
#include <numbers>

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
			//Initialize renderer color
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		}
		keyboard = SDL_GetKeyboardState(NULL);
		SDL_AddEventWatch(ResizeHandler, nullptr);

		return success;
	}

	static void DrawTexture(float windowWidth, float windowHeight) {
		SDL_SetRenderTarget(renderer, NULL);
		float scaleX = (float)windowWidth / SCREEN_WIDTH;
		float scaleY = (float)windowHeight / SCREEN_HEIGHT;
		float scale = std::min(scaleX, scaleY);  // Maintain aspect ratio

		int scaledWidth = (int)(SCREEN_WIDTH * scale);
		int scaledHeight = (int)(SCREEN_HEIGHT * scale);

		int offsetX = (windowWidth - scaledWidth) / 2;
		int offsetY = (windowHeight - scaledHeight) / 2;

		SDL_FRect dstRect = { offsetX, offsetY, scaledWidth, scaledHeight };
		SDL_RenderTexture(renderer, renderTexture, NULL, &dstRect);

		SDL_RenderPresent(renderer);
	}

	static bool SDLCALL ResizeHandler(void* userdata, SDL_Event* event) {
		if (event->type == SDL_EVENT_WINDOW_RESIZED) {
			// For window events, SDL_Event's window field holds the details.
			SDL_WindowEvent* wevent = &event->window;

			DrawTexture(wevent->data1, wevent->data2);
		}
		// Return true to let the event continue to the normal event queue.
		return true;
	}

	void SDLClose()
	{
		//Destroy window	
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		window = NULL;
		renderer = NULL;
		SDL_Quit();
	}

	//Screen dimension constants
	static const int SCREEN_WIDTH = 1920;
	static const int SCREEN_HEIGHT = 1080;

	//Keyboard
	const bool* keyboard;

	//The window we'll be rendering to
	static SDL_Window* window;

	//The window renderer
	static SDL_Renderer* renderer;

	static SDL_Texture* renderTexture;

	SDL_Color bground = { 10,18,40 };

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
				newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
				if (newTexture == NULL)
				{
					printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
				}

				//Get rid of old loaded surface
				SDL_DestroySurface(loadedSurface);
			}

			textures[path] = newTexture;
			SDL_SetTextureScaleMode(textures[path], SDL_SCALEMODE_NEAREST);
		}
	}

	Sprite GetSprite(std::string name) {
		return sprites[name];
	}
};

SDL_Window* SDLton::window = SDL_CreateWindow("Asteroids", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
SDL_Renderer* SDLton::renderer = SDL_CreateRenderer(SDLton::window, NULL);
SDL_Texture* SDLton::renderTexture = SDL_CreateTexture(
	SDLton::renderer,
	SDL_PIXELFORMAT_RGBA8888,
	SDL_TEXTUREACCESS_TARGET,
	SDLton::SCREEN_WIDTH, SDLton::SCREEN_HEIGHT
);

struct Transform {
	Vector2 position;
	Vector2 velocity;
	float rotation;
};

struct SpriteRenderer {
	Vector2 pos;
	std::string sprite;
};

class EventSystem : public System {
	void Update() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				QuitGame();
			}
		}
	}
};

class MovementSystem : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();

		for (auto object : ObjectsWith<Transform>()) {
			auto& xform = object.GetComponent<Transform>();
			if (sdl.keyboard[SDL_SCANCODE_RIGHT]) {
				object.GetComponent<Transform>().rotation+=0.02;
			}if (sdl.keyboard[SDL_SCANCODE_LEFT]) {
				object.GetComponent<Transform>().rotation-=0.02;
			}if (sdl.keyboard[SDL_SCANCODE_UP]) {
				xform.velocity += Vector2{cosf((xform.rotation-90)*std::numbers::pi/180), sinf((xform.rotation-90)*std::numbers::pi/180)} * 0.00001;
			}if (sdl.keyboard[SDL_SCANCODE_DOWN]) {
				xform.velocity -= Vector2{ cosf((xform.rotation - 90) * std::numbers::pi / 180), sinf((xform.rotation - 90) * std::numbers::pi / 180) } *0.00001;
			}

			if (xform.velocity.mag() > 0.05) {
				xform.velocity = Vector2::Unit(xform.velocity.angle())*0.05;
			}

			object.GetComponent<Transform>().position += object.GetComponent<Transform>().velocity;
		}
	}
};

class RenderSys : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();

		SDL_SetRenderTarget(sdl.renderer, sdl.renderTexture);
		
		SDL_SetRenderDrawColor(sdl.renderer, sdl.bground.r, sdl.bground.g, sdl.bground.b, 255);
		SDL_RenderClear(sdl.renderer);

		for (auto object : ObjectsWith<SpriteRenderer>()) {
			auto& spriteRenderer = object.GetComponent<SpriteRenderer>();
			auto& xform = object.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			const SDL_FRect rect = {(float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			const SDL_FRect destRect = { rect.x + xform.position.x,rect.y + xform.position.y,rect.w*4,rect.h*4 };
			SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect,xform.rotation, NULL,SDL_FLIP_NONE);

		}

		int windowWidth, windowHeight;
		SDL_GetWindowSize(sdl.window, &windowWidth, &windowHeight);
		sdl.DrawTexture(windowWidth, windowHeight);
	}
};

class AsteroidsScene : public Scene {
	void Init() {
		RegisterComponents<Transform, SpriteRenderer>();
		RegisterSystems<EventSystem,MovementSystem,RenderSys>();

		DefineObject<Transform,SpriteRenderer>("ship");

		Object o = CreateObject("ship");
		o.GetComponent<Transform>().position = { 200,200 };
		o.GetComponent<Transform>().velocity = { 0,0 };
		o.GetComponent<SpriteRenderer>().pos = { 0,0 };
		o.GetComponent<Transform>().rotation = 0;
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

int main(int argc, char** argv) {
	AsteroidsGame game;
	game.Start("AsteroidsScene");

	return 0;
}