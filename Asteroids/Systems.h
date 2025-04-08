#pragma once
#include "ECSLib.h"
#include "Components.h"

class EventSystem : public System {
public:
    void Update() override;
};

// Asteroid spawn system: spawns asteroids and updates phases.
class AsteroidSpawnSystem : public System {
public:
    void Update() override;
};

// Destroy system: checks destroy timers and destroys objects.
class DestroySystem : public System {
public:
    void Update() override;
};

// Asteroid containment system: keeps asteroids within bounds and handles collisions.
class AsteroidContainmentSystem : public System {
public:
    void Update() override;
};

// Score system: updates on-screen score text.
class ScoreSystem : public System {
public:
    void Update() override;
};

// Instructions system: handles instruction messages (e.g. game over, phase indicators).
class InstructionsSystem : public System {
public:
    void Update() override;
};

// Bullet system: destroys out-of-bound bullets.
class BulletSystem : public System {
public:
    void Update() override;
};

// Movement system: updates ship movement, input, and shooting.
class MovementSystem : public System {
public:
    void Update() override;
};

// Physics system: applies velocity updates to positions.
class PhysicsSystem : public System {
public:
    void Update() override;
};

// Render system: renders sprites for bullets, asteroids, ships, explosions.
class RenderSys : public System {
public:
    void Render(Transform& transform, SpriteRenderer& spriteRenderer);
    void Update() override;
};

// Text render system: renders text messages and manages render target swapping.
class TextRenderSystem : public System {
public:
    void Update() override;
};