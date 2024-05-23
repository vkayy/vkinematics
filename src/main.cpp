#include <SFML/Graphics.hpp>

#include "simulation/simulation.hpp"

constexpr bool RENDER_DISPLAY = true;
constexpr int32_t WINDOW_WIDTH = 1500;
constexpr int32_t WINDOW_HEIGHT = 1000;

constexpr float MAX_OBJECT_COUNT = 1000;
constexpr float MIN_RADIUS = 10.0f;
constexpr float MAX_RADIUS = 10.0f;
constexpr float MAX_ANGLE = M_PI / 2;

constexpr bool SPEED_COLOURING = true;
constexpr float SPAWN_DELAY = 0.005f;
constexpr float SPAWN_SPEED = 10.0f;

constexpr int32_t FRAMERATE_LIMIT = 60;
constexpr int32_t THREAD_COUNT = 4;
constexpr int32_t SUBSTEPS = 8;

constexpr int8_t COLLISION_RESOLVER = 0;

bool GRAVITY_ON = false;

const sf::Vector2f SPAWN_POSITION = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

int main() {
    runSimulation(
        RENDER_DISPLAY,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        MAX_OBJECT_COUNT,
        MIN_RADIUS,
        MAX_RADIUS,
        MAX_ANGLE,
        SPEED_COLOURING,
        SPAWN_DELAY,
        SPAWN_SPEED,
        FRAMERATE_LIMIT,
        THREAD_COUNT,
        SUBSTEPS,
        COLLISION_RESOLVER,
        GRAVITY_ON,
        SPAWN_POSITION
    );
    return 0;
}
