#include "Systems.h"
#include "SDL3/SDL.h"
#include "Singletons.h"
#include "Interfaces.h"

// Event System: 
// Used for polling and handling events from SDL library.
void EventSystem::Update() {
    auto& sdl = GetPersistentSingleton<SDLton>();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Quit game if X button pressed on the game window
        if (event.type == SDL_EVENT_QUIT) {
            QuitGame();
        }
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            // Check if F11 was pressed to toggle fullscreen
            if (event.key.scancode == SDL_SCANCODE_F11) {
                Uint32 flags = SDL_GetWindowFlags(sdl.window);
                if (flags & SDL_WINDOW_FULLSCREEN) {
                    SDL_SetWindowFullscreen(sdl.window, 0);
                }
                else {
                    SDL_SetWindowFullscreen(sdl.window, SDL_WINDOW_FULLSCREEN);
                }
            } 
            // Quit game if escape pressed
            else if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                QuitGame();
            }
        }
    }
}

// Asteroid Spawn System: 
// Generates new asteroids at calculated intervals.
void AsteroidSpawnSystem::Update() {
    auto& gen = GetSingleton<AsteroidGeneration>();
    // Generate new asteroid when counter is ready
    if (gen.nextAsteroidCounter == gen.nextAsteroidAt) {
        float xVel = 0;
        float yVel = 0;
        // Ensure nonzero velocity components.
        while (xVel == 0 || yVel == 0) {
            xVel = (float)(rand() % 50 - 25) / 10;
            yVel = (float)(rand() % 50 - 25) / 10;
        }
        // Create size from 0 to 2 (small to large)
        int size = rand() % 3;
        // Create an asteroid via the ObjectCreatorInterface interface.
        GetInterface<ObjectCreatorInterface>().CreateAsteroid({ -500, -500 }, { xVel, yVel }, size);
        // Reset timer for the next asteroid
        gen.nextAsteroidCounter = 0;
        gen.nextStageCounter++;
    }

    // When the "phase" counter is ready...
    if (gen.nextStageCounter == gen.nextStageAt) {
        // Increase generation rate by specific amount (chosen to play well)
        gen.nextAsteroidAt = std::ceil(gen.nextAsteroidAt * 3 / 4.0f);
        gen.nextStageAt = std::ceil(gen.nextStageAt * 4 / 3.0f);
        gen.nextStageCounter = 0;
        GetInterface<ObjectCreatorInterface>().CreateInstructions({ 1920 / 2 - 110, 1080 / 2 }, "Phase " + std::to_string(gen.stageNum), 240);
        gen.stageNum++;
    }
    gen.nextAsteroidCounter++;
}

// Destroy System:
// Destroys certain objects after their set duration.
void DestroySystem::Update() {
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

// AsteroidContainmentSystem:
// System used for keeping asteroids within certain bounds, as well 
// as checking for collisions.
void AsteroidContainmentSystem::Update() {
    for (auto asteroid : ObjectsWith("asteroid")) {
        auto& xform = asteroid.GetComponent<Transform>();

        // Move asteroid to other side of box if too far
        if (xform.position.x > 4000)
            xform.position.x = -1000;
        else if (xform.position.x < -1000)
            xform.position.x = 4000;
        if (xform.position.y > 3000)
            xform.position.y = -1000;
        else if (xform.position.y < -1000)
            xform.position.y = 3000;

        // Get the size in pixels of the asteroid
        int size;
        if (asteroid.GetComponent<Asteroid>().size == 2)
            size = 98;
        else if (asteroid.GetComponent<Asteroid>().size == 1)
            size = 64;
        else
            size = 16;

        // Only check collisions for asteroids near the screen
        if (xform.position.x > -150 && xform.position.x < 2050 &&
            xform.position.y > -150 && xform.position.y < 1250) {
            // Check collision with ship
            for (auto ship : ObjectsWith("ship")) {
                auto& shipxform = ship.GetComponent<Transform>();
                //End the game if collision with ship
                if ((shipxform.position - xform.position).mag() < size + 38) {
                    DestroyObject(ship);
                    GetInterface<ObjectCreatorInterface>().CreateInstructions({ 1920 / 2 - 160, 1080 / 2 }, "GAME OVER", 500);
                    continue;
                }
            }
            // Check collision with bullet(s)
            for (auto bullet : ObjectsWith("bullet")) {
                auto& bulletxform = bullet.GetComponent<Transform>();
                //Apply explosion force to ship from close impact
                if ((bulletxform.position - xform.position).mag() < size + 16) {
                    for (auto ship : ObjectsWith("ship")) {
                        auto& shipxform = ship.GetComponent<Transform>();
                        auto& shipcomp = ship.GetComponent<Ship>();
                        shipcomp.reloadTimer = 0;
                        float dist = (shipxform.position - bulletxform.position).mag();
                        if (dist < 600) {
                            shipxform.velocity +=
                                Vector2::Unit((shipxform.position - bulletxform.position).angle())
                                * 0.2f / std::pow(dist / 500.f, 2);
                        }
                    }
                    // Create the explosion sprite
                    Object e = CreateObject("explosion");
                    e.GetComponent<Transform>().position = xform.position;
                    e.GetComponent<SpriteRenderer>().sprite = "explosion";
                    e.GetComponent<DestroyTimer>().countdown = 20;
                    AddTag(e, "explosion");
                    DestroyObject(asteroid);
                    DestroyObject(bullet); 
                    // Advance the score
                    for (Object s : ObjectsWith<Score>()) {
                        s.GetComponent<Score>().score++;
                    }
                }
            }
        }
    }
}

// Score System:
// Updates the score text.
void ScoreSystem::Update() {
    for (auto object : ObjectsWith<Score>()) {
        auto& score = object.GetComponent<Score>();
        auto& text = object.GetComponent<TextRenderer>();

        text.message = "Score: " + std::to_string(score.score);
    }
}

// Instructions System:
// Handles the flashing instructional text messages in the center of the screen.
void InstructionsSystem::Update() {
    for (auto object : ObjectsWith("instructions")) {
        auto& tx = object.GetComponent<TextRenderer>();
        auto& i = object.GetComponent<InstructionsTimer>();
        
        // Toggle the text's visibility to create flashing effect
        if (i.timer > 0) {
            tx.visible = (i.timer % 50) > 10;
            i.timer--;
        }
        else {
            // Destroy the message after time expires, create further instructions (or exit game) if needed
            SDL_DestroyTexture(tx.texture);
            if (tx.message == "Press UP, DOWN, LEFT, RIGHT to fly.") {
                GetInterface<ObjectCreatorInterface>().CreateInstructions({ 1920 / 2 - 350, 1080 / 2 }, "Press SPACE to shoot.", 300);
            }
            else if (tx.message == "GAME OVER") {
                QuitGame();
            }
            DestroyObject(object);
        }
    }
}

// Bullet System: 
// Destroys bullets after exiting the screen.
void BulletSystem::Update() {
    for (auto bullet : ObjectsWith("bullet")) {
        auto& xform = bullet.GetComponent<Transform>();

        if (xform.position.x < -200 || xform.position.x > 2100 ||
            xform.position.y < -200 || xform.position.y > 1100) {
            DestroyObject(bullet);
        }
    }
}

// Movement System:
// System for controlling the ship using directional keyboard input.
void MovementSystem::Update() {
    auto& sdl = GetPersistentSingleton<SDLton>();

    for (auto object : ObjectsWith("ship")) {
        auto& xform = object.GetComponent<Transform>();
        auto& sr = object.GetComponent<SpriteRenderer>();
        auto& ship = object.GetComponent<Ship>();

        // Apply angular velocity for turning
        if (sdl.keyboard[SDL_SCANCODE_RIGHT] || sdl.keyboard[SDL_SCANCODE_D]) {
            xform.angularVelocity += 0.5f;
        }
        if (sdl.keyboard[SDL_SCANCODE_LEFT] || sdl.keyboard[SDL_SCANCODE_A]) {
            xform.angularVelocity -= 0.5f;
        }
        // Apply "friction" to turn if not actively turning
        if ((sdl.keyboard[SDL_SCANCODE_LEFT] || sdl.keyboard[SDL_SCANCODE_A]) ==
            (sdl.keyboard[SDL_SCANCODE_RIGHT] || sdl.keyboard[SDL_SCANCODE_D])) {
            xform.angularVelocity *= 0.85f;
        }

        // Apply linear thrusting velocity for moving forwards/ backwards
        if (sdl.keyboard[SDL_SCANCODE_UP] || sdl.keyboard[SDL_SCANCODE_W]) {
            xform.velocity += Vector2{
                std::cosf((xform.rotation - 90) * std::numbers::pi / 180),
                std::sinf((xform.rotation - 90) * std::numbers::pi / 180)
            } *0.2f;
            sr.sprite = "ship_accel";
        }
        if (sdl.keyboard[SDL_SCANCODE_DOWN] || sdl.keyboard[SDL_SCANCODE_S]) {
            xform.velocity -= Vector2{
                std::cosf((xform.rotation - 90) * std::numbers::pi / 180),
                std::sinf((xform.rotation - 90) * std::numbers::pi / 180)
            } *0.2f;
            sr.sprite = "ship_reverse";
        }

        // Cap the linear velocity if too large
        if (xform.velocity.mag() > 7) {
            xform.velocity = Vector2::Unit(xform.velocity.angle()) * 7;
        }

        // Apply slight "friction" to linear velocity
        xform.velocity *= 0.99f;

        // Zero out velocity and clamp position near the screen edges.
        if (xform.position.x < 96 || xform.position.x > 1920 - 96)
            xform.velocity.x = 0;
        if (xform.position.y < 96 || xform.position.y > 1080 - 96)
            xform.velocity.y = 0;
        xform.position.x = std::clamp(xform.position.x, 96.f, 1920.f - 96);
        xform.position.y = std::clamp(xform.position.y, 96.f, 1080.f - 96);

        // Cap the angular velocity if too large
        xform.angularVelocity = std::clamp(xform.angularVelocity, -5.f, 5.f);

        // Update the rotation by angular velocity
        xform.rotation += xform.angularVelocity;

        // Set the ship's sprite depending on its state
        if (ship.reloadTimer > 0) {
            sr.sprite = (sr.sprite == "ship") ? "ship_reloading" :
                (sr.sprite == "ship_accel") ? "ship_accel_reloading" :
                "ship_reverse_reloading";
        }

        // Fire a rocket if SPACE pressed and rocket available
        if (sdl.keyboard[SDL_SCANCODE_SPACE] && ship.reloadTimer == 0 && !ship.spaceHeld) {
            xform.velocity -= Vector2{
                std::cosf((xform.rotation - 90) * std::numbers::pi / 180),
                std::sinf((xform.rotation - 90) * std::numbers::pi / 180)
            } *0.8f;
            GetInterface<ObjectCreatorInterface>().CreateBullet(xform.position, xform.velocity, xform.rotation);
            ship.reloadTimer = 60;
        }
        // Decrement reload timer
        else if (ship.reloadTimer > 0) {
            ship.reloadTimer--;
        }
        // Track the state of SPACE key press
        if (sdl.keyboard[SDL_SCANCODE_SPACE]) {
            ship.spaceHeld = true;
        }
        else {
            ship.spaceHeld = false;
        }
    }
}

// Physics System:
// Updates objects' positions by their velocities.
void PhysicsSystem::Update() {
    for (auto object : ObjectsWith<Transform>()) {
        object.GetComponent<Transform>().position += object.GetComponent<Transform>().velocity;
    }
}

// Render System:
// Renders all visible objects to the screen.
void RenderSys::Render(Transform& transform, SpriteRenderer& spriteRenderer) {      // Helper function for rendering an object to the screen
    auto& sdl = GetPersistentSingleton<SDLton>();

    auto sprite = sdl.GetSprite(spriteRenderer.sprite);
    const SDL_FRect rect = { float(sprite.clip.x), float(sprite.clip.y),
                             float(sprite.clip.w), float(sprite.clip.h) };
    const SDL_FRect destRect = { transform.position.x - rect.w * 2,
                                 transform.position.y - rect.h * 2,
                                 rect.w * 4, rect.h * 4 };
    SDL_RenderTextureRotated(sdl.renderer, sprite.texture, &rect, &destRect, transform.rotation, nullptr, SDL_FLIP_NONE);
}

// Render all visible objects to the screen.
void RenderSys::Update() {      
    // Render to a single texture before rendering to the screen 
    auto& sdl = GetPersistentSingleton<SDLton>();
    SDL_SetRenderTarget(sdl.renderer, sdl.renderTexture);

    // Clear and prepare the renderer
    SDL_SetRenderDrawColor(sdl.renderer, sdl.bground.r, sdl.bground.g, sdl.bground.b, 255);
    SDL_RenderClear(sdl.renderer);

    //Render objects by type so that they are rendered in the correct order:
    // Render bullets.
    for (auto bullet : ObjectsWith("bullet")) {
        auto& spriteRenderer = bullet.GetComponent<SpriteRenderer>();
        auto& xform = bullet.GetComponent<Transform>();
        Render(xform, spriteRenderer);
    }
    // Render asteroids.
    for (auto asteroid : ObjectsWith("asteroid")) {
        auto& spriteRenderer = asteroid.GetComponent<SpriteRenderer>();
        auto& xform = asteroid.GetComponent<Transform>();
        Render(xform, spriteRenderer);
    }
    // Render ships.
    for (auto ship : ObjectsWith("ship")) {
        auto& spriteRenderer = ship.GetComponent<SpriteRenderer>();
        auto& xform = ship.GetComponent<Transform>();
        Render(xform, spriteRenderer);
    }
    // Render explosions.
    for (auto explosion : ObjectsWith("explosion")) {
        auto& spriteRenderer = explosion.GetComponent<SpriteRenderer>();
        auto& xform = explosion.GetComponent<Transform>();
        Render(xform, spriteRenderer);
    }
}

// Text Render System:
// Renders text to the screen and automatically updates textures when the message changes.
void TextRenderSystem::Update() {
    auto& sdl = GetPersistentSingleton<SDLton>();

    for (auto object : ObjectsWith<TextRenderer>()) {
        auto& tx = object.GetComponent<TextRenderer>();
        // Check if text is visible...
        if (tx.visible) {
            // Load new message if message changed
            if (tx.message != tx.currentMessage) {
                if (tx.texture) {
                    SDL_DestroyTexture(tx.texture);
                    tx.texture = nullptr;
                }
                tx.texture = sdl.LoadText(tx.message);
                tx.currentMessage = tx.message;
            }

            // Render text
            auto& xform = object.GetComponent<Transform>();
            float w, h;
            SDL_GetTextureSize(tx.texture, &w, &h);
            SDL_FRect dstRect = { xform.position.x, xform.position.y, w, h };
            SDL_RenderTexture(sdl.renderer, tx.texture, nullptr, &dstRect);
        }
    }

    // Render the intermediate texture to the screen. This allows the aspect ratio to be maintained.
    SDL_SetRenderTarget(sdl.renderer, nullptr);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(sdl.window, &windowWidth, &windowHeight);
    float scaleX = static_cast<float>(windowWidth) / sdl.SCREEN_WIDTH;
    float scaleY = static_cast<float>(windowHeight) / sdl.SCREEN_HEIGHT;
    float scale = std::min(scaleX, scaleY);

    int scaledWidth = static_cast<int>(sdl.SCREEN_WIDTH * scale);
    int scaledHeight = static_cast<int>(sdl.SCREEN_HEIGHT * scale);

    int offsetX = (windowWidth - scaledWidth) / 2;
    int offsetY = (windowHeight - scaledHeight) / 2;

    SDL_FRect dstRect = { static_cast<float>(offsetX), static_cast<float>(offsetY),
                          static_cast<float>(scaledWidth), static_cast<float>(scaledHeight) };
    SDL_RenderTexture(sdl.renderer, sdl.renderTexture, nullptr, &dstRect);
    SDL_RenderPresent(sdl.renderer);
}