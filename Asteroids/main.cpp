#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "ECSLib.h"
#include <random>
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
			window = SDL_CreateWindow("Asteroids", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
			renderer = SDL_CreateRenderer(SDLton::window, NULL);
			//Initialize renderer color
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			renderTexture = SDL_CreateTexture(
				SDLton::renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET,
				SDLton::SCREEN_WIDTH, SDLton::SCREEN_HEIGHT
			);
			if (TTF_Init() == -1) {
				fprintf(stderr, "TTF_Init Error: %s\n", SDL_GetError());
				SDL_Quit();
				return 1;
			}
			else {
				font = TTF_OpenFont("Assets/Retro.ttf", 48);
			}
		}
		keyboard = SDL_GetKeyboardState(NULL);
		SDL_SetRenderVSync(renderer, 1);

		return success;
	}

	void SDLClose()
	{
		//Destroy window	
		TTF_CloseFont(font);
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
	SDL_Window* window;

	//The window renderer
	SDL_Renderer* renderer;

	SDL_Texture* renderTexture;

	SDL_Color bground = { 10,18,40 };

	TTF_Font* font;

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
	SDL_Texture* LoadText(std::string textContent) {
		SDL_Surface* text = TTF_RenderText_Blended(font, textContent.c_str(),textContent.size(), SDL_Color{255,255,255,255});
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
	}

	Sprite GetSprite(std::string name) {
		return sprites[name];
	}
};

struct Transform {
	Transform() {
		position = Vector2();
		velocity = Vector2();
		rotation = 0;
	}

	Vector2 position = {0,0};
	Vector2 velocity = {0,0};
	float rotation=0;
};

struct SpriteRenderer {
	std::string sprite="";
};

struct TextRenderer {
	std::string message="";
	std::string currentMessage="";
	SDL_Texture* texture=nullptr;
	bool visible = true;
};

struct DestroyTimer {
	int countdown = 20;
};

struct Asteroid {
	int size=0;
};

struct Score {
	int score=0;
};

struct InstructionsTimer {
	int timer=0;
};

struct Ship {
	int shootTimer=0;
	bool spaceHeld = false;
};

class EntityCreator : public GInterface {
public:
	void CreateAsteroid(Vector2 position, Vector2 velocity, int size) {
		Object asteroid = CreateObject("asteroid");
		asteroid.GetComponent<Transform>().position = position;
		asteroid.GetComponent<Transform>().velocity = velocity;
		asteroid.GetComponent<Transform>().rotation = rand()%360;
		asteroid.GetComponent<SpriteRenderer>().sprite = size==2?"big":size==1?"med":"small";
		asteroid.GetComponent<Asteroid>().size = size;
		AddTag(asteroid, "asteroid");
	}
	void CreateBullet(Vector2 position, Vector2 velocity, float angle) {
		Object bullet = CreateObject("bullet");
		bullet.GetComponent<Transform>().position = position;
		bullet.GetComponent<Transform>().velocity = Vector2::Unit((angle-90)*std::numbers::pi/180)*40 + velocity;
		bullet.GetComponent<SpriteRenderer>().sprite = "bullet";
		bullet.GetComponent<Transform>().rotation = angle-90;
		AddTag(bullet, "bullet");
	}
};

class EventSystem : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				QuitGame();
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				// Check if F11 was pressed (use SDL_SCANCODE_F11 or SDLK_F11)
				if (event.key.scancode == SDL_SCANCODE_F11) {
					// Get current fullscreen state
					Uint32 flags = SDL_GetWindowFlags(sdl.window);
					if (flags & SDL_WINDOW_FULLSCREEN) {
						// If currently fullscreen, switch to windowed mode
						SDL_SetWindowFullscreen(sdl.window, 0);
					}
					else {
						// Else, set fullscreen desktop mode
						SDL_SetWindowFullscreen(sdl.window, SDL_WINDOW_FULLSCREEN);
					}
				}
			}
		}
	}
};

class AsteroidSpawnSystem : public System {
	int nextAsteroidCounter = 0;
	int nextAsteroidAt = 180;
	int nextStageCounter = 0;
	int nextStageAt = 20;
	int stageNum = 1;
	void Update() {
		if (nextAsteroidCounter == nextAsteroidAt) {
			float xVel = 0; 
			float yVel = 0;
			while (xVel == 0 || yVel == 0) {
				xVel = (float)(rand() % 50 - 25) / 10;
				yVel = (float)(rand() % 50 - 25) / 10;
			}
			int size = rand() % 3;
			GetInterface<EntityCreator>().CreateAsteroid({ -500,-500 }, { xVel,yVel }, size);
			nextAsteroidCounter = 0;
			nextStageCounter++;
		}
		if (nextStageCounter == nextStageAt) {
			nextAsteroidAt = std::ceil(nextAsteroidAt * 3 / 4);
			nextStageAt = std::ceil(nextStageAt * 4/3);
			nextStageCounter = 0;
			Object i = CreateObject("instructions");
			i.GetComponent<Transform>().position = { 1920 / 2 - 110,1080 / 2 };
			i.GetComponent<TextRenderer>().message = "Phase "+std::to_string(stageNum);
			i.GetComponent<InstructionsTimer>().timer = 240;
			AddTag(i, "instructions");
			stageNum++;
		}
		nextAsteroidCounter++;
	}
};

class DestroySystem : public System {
	void Update() {
		for (Object object : ObjectsWith<DestroyTimer>()) {
			auto& timer = object.GetComponent<DestroyTimer>();

			if (timer.countdown <= 0) {
				DestroyObject(object);
			}
			else {
				timer.countdown--;
			}
		}
	}
};

class AsteroidContainmentSystem : public System {
	void Update() {
		for (auto asteroid : ObjectsWith("asteroid")) {
			auto& xform = asteroid.GetComponent<Transform>();

			if (xform.position.x > 4000) {
				xform.position.x = -1000;
			}
			else if (xform.position.x < -1000) {
				xform.position.x = 4000;
			}
			if (xform.position.y > 3000) {
				xform.position.y = -1000;
			}
			else if (xform.position.y < -1000) {
				xform.position.y = 3000;
			}

			int size;
			if (asteroid.GetComponent<Asteroid>().size == 2) {
				size = 98;
			}
			else if (asteroid.GetComponent<Asteroid>().size == 1) {
				size = 64;
			}
			else {
				size = 16;
			}
			
			if (xform.position.x > -150 && xform.position.x < 2050 && xform.position.y > -150 && xform.position.y < 1250) {
				for (auto ship : ObjectsWith("ship")) {
					auto& shipxform = ship.GetComponent<Transform>();

					if ((shipxform.position - xform.position).mag() < size+38) {
						DestroyObject(ship);
						Object i = CreateObject("instructions");
						i.GetComponent<Transform>().position = { 1920 / 2 - 160,1080 / 2 };
						i.GetComponent<TextRenderer>().message = "GAME OVER";
						i.GetComponent<InstructionsTimer>().timer = 1000;
						AddTag(i, "instructions");
						continue;
					}
				}
				for (auto bullet : ObjectsWith("bullet")) {
					auto& bulletxform = bullet.GetComponent<Transform>();
					if ((bulletxform.position - xform.position).mag() < size+16) {
						for (auto ship : ObjectsWith("ship")) {
							auto& shipxform = ship.GetComponent<Transform>();
							auto& shipcomp = ship.GetComponent<Ship>();
							shipcomp.shootTimer = 0;
							float dist = (shipxform.position - bulletxform.position).mag();
							if (dist < 600) {
								shipxform.velocity +=
									Vector2::Unit((shipxform.position - bulletxform.position).angle()) * 0.2 / powf(dist/500, 2);
							}
						}
						Object e = CreateObject("explosion");
						e.GetComponent<Transform>().position = xform.position;
						e.GetComponent<SpriteRenderer>().sprite = "explosion";
						e.GetComponent<DestroyTimer>().countdown = 20;
						AddTag(e, "explosion");
						DestroyObject(asteroid);
						DestroyObject(bullet);
						for (Object s : ObjectsWith<Score>()) {
							s.GetComponent<Score>().score++;
						}
					}
				}
			}
		}
	}
};

class ScoreSystem : public System {
	void Update() {
		for (auto object : ObjectsWith<Score>()) {
			auto& score = object.GetComponent<Score>();
			auto& tx = object.GetComponent<TextRenderer>();

			tx.message = "Score: " + std::to_string(score.score);
		}
	}
};

class InstructionsSystem : public System {
	int flashTimer = 120;
	void Update() {
		for (auto object : ObjectsWith("instructions")) {
			auto& tx = object.GetComponent<TextRenderer>();
			auto& i = object.GetComponent<InstructionsTimer>();

			if (i.timer > 0) {
				tx.visible = i.timer % 50 > 10;
				i.timer--;
			}
			else {
				SDL_DestroyTexture(tx.texture);
				if (tx.message == "Press UP, DOWN, LEFT, RIGHT to fly.") {
					Object i = CreateObject("instructions");
					i.GetComponent<Transform>().position = { 1920 / 2 - 350,1080 / 2 };
					i.GetComponent<TextRenderer>().message = "Press SPACE to shoot.";
					i.GetComponent<InstructionsTimer>().timer = 300;
					AddTag(i, "instructions");
				}
				else if (tx.message == "GAME OVER") {
					QuitGame();
				}
				DestroyObject(object);
			}
		}
	}
};

class BulletSystem : public System {
	void Update() {
		for (auto bullet : ObjectsWith("bullet")) {
			auto& xform = bullet.GetComponent<Transform>();

			if (xform.position.x < -200 || xform.position.x > 2100|| xform.position.y < -200|| xform.position.y > 1100) {
				DestroyObject(bullet);
			}
		}
	}
};

class MovementSystem : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();

		for (auto object : ObjectsWith("ship") ){
			auto& xform = object.GetComponent<Transform>();
			auto& sr = object.GetComponent<SpriteRenderer>();
			auto& ship = object.GetComponent<Ship>();
			sr.sprite = "ship";
			if (sdl.keyboard[SDL_SCANCODE_RIGHT]|| sdl.keyboard[SDL_SCANCODE_D]) {
				object.GetComponent<Transform>().rotation+=4;
			}
			if (sdl.keyboard[SDL_SCANCODE_LEFT]|| sdl.keyboard[SDL_SCANCODE_A]) {
				object.GetComponent<Transform>().rotation-=4;
			}
			if (sdl.keyboard[SDL_SCANCODE_UP]|| sdl.keyboard[SDL_SCANCODE_W]) {
				xform.velocity += Vector2{cosf((xform.rotation-90)*std::numbers::pi/180), sinf((xform.rotation-90)*std::numbers::pi/180)} * 0.2;
				sr.sprite = "ship_accel";
			}
			if (sdl.keyboard[SDL_SCANCODE_DOWN]|| sdl.keyboard[SDL_SCANCODE_S]) {
				xform.velocity -= Vector2{ cosf((xform.rotation - 90) * std::numbers::pi / 180), sinf((xform.rotation - 90) * std::numbers::pi / 180) } *0.2;
				sr.sprite = "ship_reverse";
			}

			if (xform.velocity.mag() > 7) {
				xform.velocity = Vector2::Unit(xform.velocity.angle())*7;
			}

			xform.velocity *= 0.99;

			xform.velocity.x = xform.position.x < 48 * 2 ? 0 : xform.position.x > 1920 - 48 * 2 ? 0 : xform.velocity.x;
			xform.velocity.y = xform.position.y < 48 * 2 ? 0 : xform.position.y > 1080 - 48 * 2 ? 0 : xform.velocity.y;

			xform.position.x = std::clamp(xform.position.x, (float)48*2, (float)1920 - 48*2);
			xform.position.y = std::clamp(xform.position.y, (float)48 *2, (float)1080 - 48*2);

			if (ship.shootTimer > 0) {
				sr.sprite = sr.sprite == "ship" ? "ship_reloading" :
					sr.sprite == "ship_accel" ? "ship_accel_reloading" :
					"ship_reverse_reloading";
			}

			if (sdl.keyboard[SDL_SCANCODE_SPACE] && ship.shootTimer == 0 && !ship.spaceHeld) {
				xform.velocity -= Vector2{ cosf((xform.rotation - 90) * std::numbers::pi / 180),
					sinf((xform.rotation - 90) * std::numbers::pi / 180) } *0.8;
				GetInterface<EntityCreator>().CreateBullet(xform.position, xform.velocity, xform.rotation);
				ship.shootTimer = 60;
			}
			else if (ship.shootTimer > 0) {
				ship.shootTimer--;
			}
			if (sdl.keyboard[SDL_SCANCODE_SPACE]) {
				ship.spaceHeld = true;
			}
			else {
				ship.spaceHeld = false;
			}
		}
	}
};

class PhysicsSystem : public System {
	void Update() {
		for (auto object : ObjectsWith<Transform>()) {
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

		for (auto bullet : ObjectsWith("bullet")) {
			auto& spriteRenderer = bullet.GetComponent<SpriteRenderer>();
			auto& xform = bullet.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			const SDL_FRect rect = { (float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			const SDL_FRect destRect = { xform.position.x - rect.w * 2,xform.position.y - rect.h * 2,rect.w * 4,rect.h * 4 };
			SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect, xform.rotation, NULL, SDL_FLIP_NONE);
		}
		for (auto asteroid : ObjectsWith("asteroid")) {
			auto& spriteRenderer = asteroid.GetComponent<SpriteRenderer>();
			auto& xform = asteroid.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			const SDL_FRect rect = { (float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			const SDL_FRect destRect = { xform.position.x- rect.w * 2,xform.position.y-rect.h * 2,rect.w * 4,rect.h * 4 };
			SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect, xform.rotation, NULL, SDL_FLIP_NONE);
		}
		for (auto ship : ObjectsWith("ship")) {
			auto& spriteRenderer = ship.GetComponent<SpriteRenderer>();
			auto& xform = ship.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			SDL_FRect rect = { (float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			SDL_FRect destRect = { xform.position.x - rect.w * 2,xform.position.y - rect.h * 2,rect.w * 4,rect.h * 4 };
			//SDL_FPoint center = { 96,100 };
			SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect, xform.rotation, NULL, SDL_FLIP_NONE);
		}
		for (auto explosion : ObjectsWith("explosion")) {
			auto& spriteRenderer = explosion.GetComponent<SpriteRenderer>();
			auto& xform = explosion.GetComponent<Transform>();

			auto sprite = sdl.GetSprite(spriteRenderer.sprite);
			const SDL_FRect rect = { (float)sprite.clip.x,(float)sprite.clip.y,(float)sprite.clip.w,(float)sprite.clip.h };
			const SDL_FRect destRect = { xform.position.x - rect.w * 2,xform.position.y - rect.h * 2,rect.w * 4,rect.h * 4 };
			SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect, xform.rotation, NULL, SDL_FLIP_NONE);
		}
	}
};

class TextRenderSystem : public System {
	void Update() {
		auto& sdl = GetPersistentSingleton<SDLton>();

		for (auto object : ObjectsWith<TextRenderer>()) {
			auto& tx = object.GetComponent<TextRenderer>();
			if (tx.visible) {
				if (tx.message != tx.currentMessage) {
					if (tx.texture) {
						SDL_DestroyTexture(tx.texture);
						tx.texture = nullptr;
					}
					tx.texture = sdl.LoadText(tx.message);
					tx.currentMessage = tx.message;
				}

				auto& xform = object.GetComponent<Transform>();
				float w, h;
				SDL_GetTextureSize(tx.texture, &w,&h);
				SDL_FRect dstRect = { xform.position.x, xform.position.y, w, h };
				SDL_RenderTexture(sdl.renderer, tx.texture, NULL, &dstRect);
			}
		}

		SDL_SetRenderTarget(sdl.renderer, NULL);
		int windowWidth, windowHeight;
		SDL_GetWindowSize(sdl.window, &windowWidth, &windowHeight);
		float scaleX = (float)windowWidth / sdl.SCREEN_WIDTH;
		float scaleY = (float)windowHeight / sdl.SCREEN_HEIGHT;
		float scale = std::min(scaleX, scaleY);  // Maintain aspect ratio

		int scaledWidth = (int)(sdl.SCREEN_WIDTH * scale);
		int scaledHeight = (int)(sdl.SCREEN_HEIGHT * scale);

		int offsetX = (windowWidth - scaledWidth) / 2;
		int offsetY = (windowHeight - scaledHeight) / 2;

		SDL_FRect dstRect = { offsetX, offsetY, scaledWidth, scaledHeight };
		SDL_RenderTexture(sdl.renderer, sdl.renderTexture, NULL, &dstRect);
		SDL_RenderPresent(sdl.renderer);

	}
};

class AsteroidsScene : public Scene {
	void Init() {
		RegisterComponents<Transform, SpriteRenderer,Asteroid,DestroyTimer,
			TextRenderer,Score,InstructionsTimer,Ship>();
		RegisterSystems<EventSystem,AsteroidSpawnSystem,AsteroidContainmentSystem,ScoreSystem,
			DestroySystem,BulletSystem,MovementSystem,PhysicsSystem,InstructionsSystem,RenderSys,TextRenderSystem>();
		CreateInterfaces<EntityCreator>();

		DefineObject<Transform,SpriteRenderer,Ship>("ship");
		DefineObject<Transform,SpriteRenderer,Asteroid>("asteroid");
		DefineObject<Transform,SpriteRenderer>("bullet");
		DefineObject<Transform,SpriteRenderer,DestroyTimer>("explosion");
		DefineObject<Transform,TextRenderer,Score>("scoreboard");
		DefineObject<Transform,TextRenderer,InstructionsTimer>("instructions");

		Object o = CreateObject("ship");
		o.GetComponent<Transform>().position = { 1920/2,1080/2 };
		o.GetComponent<Transform>().velocity = { 0,0 };
		o.GetComponent<Transform>().rotation = 0;
		o.GetComponent<SpriteRenderer>().sprite = "ship";
		AddTag(o, "ship");

		Object s = CreateObject("scoreboard");
		s.GetComponent<Transform>().position = { 20,20 };
		s.GetComponent<Score>().score = 0;
		s.GetComponent<TextRenderer>().message = "Score: 0";

		Object i = CreateObject("instructions");
		i.GetComponent<Transform>().position = { 1920 / 2-525,1080 / 2 };
		i.GetComponent<TextRenderer>().message = "Press UP, DOWN, LEFT, RIGHT to fly.";
		i.GetComponent<InstructionsTimer>().timer = 300;
		AddTag(i, "instructions");
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
		sdl.CreateSprite("ship", "Assets/ship-a1.png", { 0,0,48,48 });
		sdl.CreateSprite("ship_accel", "Assets/ship-a2.png", { 0,0,48,48 });
		sdl.CreateSprite("ship_reverse", "Assets/ship-a3.png", { 0,0,48,48 });
		sdl.CreateSprite("ship_reloading", "Assets/ship-a4.png", { 0,0,48,48 });
		sdl.CreateSprite("ship_accel_reloading", "Assets/ship-a5.png", { 0,0,48,48 });
		sdl.CreateSprite("ship_reverse_reloading", "Assets/ship-a6.png", { 0,0,48,48 });
		sdl.CreateSprite("big", "Assets/big-a.png", { 0,0,48,48 });
		sdl.CreateSprite("med", "Assets/med-a.png", { 0,0,48,48 });
		sdl.CreateSprite("small", "Assets/small-a.png", { 0,0,48,48 });
		sdl.CreateSprite("bullet", "Assets/bullet-b1.png", { 0,0,16,16 });
		sdl.CreateSprite("explosion", "Assets/explosions-a5.png", { 0,0,32,32 });

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