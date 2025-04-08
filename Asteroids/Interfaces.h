#pragma once
#include "ECSLib.h"
#include "Components.h"
#include <numbers>

class ObjectCreatorInterface : public GInterface {
public:
	// Create an asteroid object given its position, velocity, and size
	void CreateAsteroid(Vector2 position, Vector2 velocity, int size) {
		Object asteroid = CreateObject("asteroid");
		asteroid.GetComponent<Transform>().position = position;
		asteroid.GetComponent<Transform>().velocity = velocity;
		asteroid.GetComponent<Transform>().rotation = rand() % 360;
		asteroid.GetComponent<SpriteRenderer>().sprite = size == 2 ? "big" : size == 1 ? "med" : "small";
		asteroid.GetComponent<Asteroid>().size = size;
		AddTag(asteroid, "asteroid");
	}
	// Create a bullet object given its position, velocity, and angle
	void CreateBullet(Vector2 position, Vector2 velocity, float angle) {
		Object bullet = CreateObject("bullet");
		bullet.GetComponent<Transform>().position = position;
		bullet.GetComponent<Transform>().velocity = Vector2::Unit((angle - 90) * std::numbers::pi / 180) * 40 + velocity;
		bullet.GetComponent<SpriteRenderer>().sprite = "bullet";
		bullet.GetComponent<Transform>().rotation = angle - 90;
		AddTag(bullet, "bullet");
	}
	// Create an instructions object given its position, message content, and duration
	void CreateInstructions(Vector2 position, std::string message, int timer) {
		Object i = CreateObject("instructions");
		i.GetComponent<Transform>().position = position;
		i.GetComponent<TextRenderer>().message = message;
		i.GetComponent<InstructionsTimer>().timer = timer;
		AddTag(i, "instructions");
	}
};