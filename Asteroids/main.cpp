#include "SDL3/SDL_main.h"
#include "ECSLib.h"
#include "Vector2.h"
#include "Components.h"
#include "Interfaces.h"
#include "Singletons.h"
#include "Systems.h"

// This is the "Scene" where the game is played. Components, interfaces, and systems must be registered
// to be part of the scene. 
class AsteroidsScene : public Scene {
	// This is the initialization function used to register the needed classes for the scene. Object types 
	// are then defined, and the starting objects can be created.
	void Init() {
		RegisterComponents<Transform, SpriteRenderer,Asteroid,DestroyTimer,
			TextRenderer,Score,InstructionsTimer,Ship>();
		RegisterSystems<EventSystem,AsteroidSpawnSystem,AsteroidContainmentSystem,ScoreSystem,
			DestroySystem,BulletSystem,MovementSystem,PhysicsSystem,InstructionsSystem,RenderSys,TextRenderSystem>();
		CreateInterfaces<ObjectCreatorInterface>();
		CreateSingleton<AsteroidGeneration>();

		DefineObject<Transform,SpriteRenderer,Ship>("ship");
		DefineObject<Transform,SpriteRenderer,Asteroid>("asteroid");
		DefineObject<Transform,SpriteRenderer>("bullet");
		DefineObject<Transform,SpriteRenderer,DestroyTimer>("explosion");
		DefineObject<Transform,TextRenderer,Score>("scoreboard");
		DefineObject<Transform,TextRenderer,InstructionsTimer>("instructions");

		Object ship = CreateObject("ship");
		ship.GetComponent<Transform>().position = { 1920/2,1080/2 };
		ship.GetComponent<Transform>().velocity = { 0,0 };
		ship.GetComponent<Transform>().rotation = 0;
		ship.GetComponent<SpriteRenderer>().sprite = "ship";
		AddTag(ship, "ship");

		Object score = CreateObject("scoreboard");
		score.GetComponent<Transform>().position = { 20,20 };
		score.GetComponent<Score>().score = 0;
		score.GetComponent<TextRenderer>().message = "Score: 0";

		Object instructions = CreateObject("instructions");
		instructions.GetComponent<Transform>().position = { 1920 / 2-525,1080 / 2 };
		instructions.GetComponent<TextRenderer>().message = "Press UP, DOWN, LEFT, RIGHT to fly.";
		instructions.GetComponent<InstructionsTimer>().timer = 300;
		AddTag(instructions, "instructions");
	}
};

// This is the Game class representing the runnable application. Inside, we can create and initialize persistent
// Singletons, as well as register the scenes used by the game.
class AsteroidsGame : public Game {
protected:
	void Init() {
		// Create a Singleton for rendering data, accessible across Scenes
		CreatePersistentSingletons<SDLton>();
		auto& sdl = GetPersistentSingleton<SDLton>();
		// Access and initialize the renderer Singleton
		sdl.SDLInit();

		//Register scenes
		RegisterScene<AsteroidsScene>("AsteroidsScene");
	}

	// Close the renderer and free memory on application exit
	void Quit() {
		GetPersistentSingleton<SDLton>().SDLClose();
	}
};

int main(int argc, char** argv) {
	// Create and start the game
	AsteroidsGame game;
	game.Start("AsteroidsScene");

	return 0;
}