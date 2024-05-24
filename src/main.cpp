#include <SFML/Graphics.hpp>

#include "simulation/simulation.hpp"

constexpr bool RENDER_DISPLAY = true;
constexpr int32_t WINDOW_WIDTH = 1500;
constexpr int32_t WINDOW_HEIGHT = 1000;

constexpr float MIN_RADIUS = 10.0f;
constexpr float MAX_RADIUS = 10.0f;

constexpr bool SPEED_COLOURING = true;
constexpr float SPAWN_DELAY = 0.005f;
constexpr float SPAWN_SPEED = 10.0f;

constexpr int32_t FRAMERATE_LIMIT = 60;
constexpr int32_t THREAD_COUNT = 3;
constexpr int32_t SUBSTEPS = 8;

constexpr int8_t COLLISION_RESOLVER = 0;

bool GRAVITY_ON = false;

const sf::Vector2f SPAWN_POSITION = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

const std::string NAME = "Multithreaded Physics Engine";

int main() {
    Simulation simulation{
        RENDER_DISPLAY,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        MIN_RADIUS,
        MAX_RADIUS,
        SPEED_COLOURING,
        FRAMERATE_LIMIT,
        THREAD_COUNT,
        SUBSTEPS,
        COLLISION_RESOLVER,
        GRAVITY_ON,
        NAME
    };
    simulation.enqueueSpawn(
        100,
        {0.5f, 0.5f},
        10.0f,
        0.005f,
        1.0f
    );
    simulation.enqueueSpawn(
        20,
        {0.5f, 0.5f},
        10.0f,
        0.005f,
        1.0f
    );
    simulation.run();
    return 0;
}
