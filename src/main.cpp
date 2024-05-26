#include <SFML/Graphics.hpp>

#include "simulation/simulation.hpp"

constexpr bool RENDER_DISPLAY = true;
constexpr int32_t WINDOW_WIDTH = 1500;
constexpr int32_t WINDOW_HEIGHT = 1000;

constexpr float MIN_RADIUS = 10.0f;
constexpr float MAX_RADIUS = 10.0f;

constexpr bool SPEED_COLOURING = true;

constexpr int32_t MAX_OBJECT_COUNT = 10000;
constexpr int32_t FRAMERATE_LIMIT = 60;
constexpr int32_t THREAD_COUNT = 3;
constexpr int32_t SUBSTEPS = 8;

constexpr int8_t COLLISION_RESOLVER = 0;

bool GRAVITY_ON = true;

const std::string NAME = "Multithreaded Physics Engine";

int main() {
    Simulation simulation{
        RENDER_DISPLAY,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        MIN_RADIUS,
        MAX_RADIUS,
        SPEED_COLOURING,
        MAX_OBJECT_COUNT,
        FRAMERATE_LIMIT,
        THREAD_COUNT,
        SUBSTEPS,
        COLLISION_RESOLVER,
        GRAVITY_ON,
        NAME
    };
    /*
    simulation.spawnRope(
        length,
        spawn_position,
        spawn_delay,
        radius
    );
    simulation.spawnFree(
        count,
        spawn_position,
        spawn_speed,
        spawn_delay,
        spawn_angle
    );
    simulation.spawnSoftBody(
        spawn_position,
        size,
        squish_factor
    )
    simulation.spawnSquare(
        spawn_positin,
        side_length
    )
    */
    simulation.spawnRope(
        20,
        {0.2f, 0.6f},
        0.005f,
        10.0f
    );
    simulation.spawnFree(
        10,
        {0.5f, 0.5f},
        10.0f,
        0.005f,
        1.0f
    );
    simulation.spawnSoftBody(
        {0.9, 0.8f},
        4.0f,
        0.5f
    );
    simulation.spawnSquare(
        {0.1, 0.3f},
        40.0f
    );
    simulation.spawnSoftBody(
        {0.9, 0.8f},
        8.0f,
        0.2f
    );
    simulation.spawnSquare(
        {0.1, 0.8f},
        100.0f
    );
    simulation.spawnRope(
        20,
        {0.8f, 0.6f},
        0.005f,
        10.0f
    );
    simulation.idle();
    return 0;
}
