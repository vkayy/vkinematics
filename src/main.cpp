#include <SFML/Graphics.hpp>

#include "simulation/simulation.hpp"

constexpr int32_t WINDOW_WIDTH = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

constexpr float MAX_OBJECT_COUNT = 1500;
constexpr float MIN_RADIUS = 10.0f;
constexpr float MAX_RADIUS = 15.0f;
constexpr float MAX_ANGLE = 1.0f;

constexpr bool SPEED_COLOURING = true;
constexpr float SPAWN_DELAY = 0.005f;
constexpr float SPAWN_SPEED = 10.0f;

constexpr int32_t FRAMERATE_LIMIT = 60;
constexpr int32_t THREAD_COUNT = 10;
constexpr int32_t SUBSTEPS = 8;

constexpr int8_t COLLISION_RESOLVER = 0;

const sf::Vector2f SPAWN_POSITION = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

int main() {
    runSimulation(
        WINDOW_HEIGHT,
        WINDOW_WIDTH,
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
        SPAWN_POSITION
    );
    return 0;
}
