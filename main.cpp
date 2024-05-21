#include <iostream>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "solver.hpp"
#include "renderer.hpp"
#include "thread_pool.hpp"
#include "utils/number_generator.hpp"

constexpr int32_t WINDOW_WIDTH = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;
constexpr float MIN_RADIUS = 10.0f;
constexpr float MAX_RADIUS = 15.0f;
constexpr float MAX_OBJECT_COUNT = 2000;
constexpr float MAX_ANGLE = 1.0f;
constexpr int32_t FRAMERATE_LIMIT = 60;
constexpr bool SPEED_COLOURING = true;
constexpr int32_t SUBSTEPS = 8;
constexpr float SPAWN_DELAY = 0.005f;
constexpr float SPAWN_SPEED = 10.0f;
const int32_t THREAD_COUNT = 10;
const sf::Vector2f SPAWN_POSITION = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

static sf::Color getRainbow(float t)
{
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * M_PI);
    const float b = sin(t + 0.66f * 2.0f * M_PI);
    return {
        static_cast<uint8_t>(255.0f * r * r),
        static_cast<uint8_t>(255.0f * g * g),
        static_cast<uint8_t>(255.0f * b * b)
    };
}

int main()
{
    constexpr int32_t window_width = WINDOW_WIDTH;
    constexpr int32_t window_height = WINDOW_HEIGHT;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 10;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Multithreaded Physics Simulator", sf::Style::Default, settings);

    
    tp::ThreadPool thread_pool(THREAD_COUNT);

    Solver solver(
        sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT),
        SUBSTEPS,
        std::max(MAX_RADIUS, WINDOW_WIDTH / 30.0f),
        FRAMERATE_LIMIT,
        SPEED_COLOURING,
        thread_pool
    );
    Renderer renderer{window};

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            } else {
                solver.setAttractor(sf::Keyboard::isKeyPressed(sf::Keyboard::A));
                solver.setRepellor(sf::Keyboard::isKeyPressed(sf::Keyboard::R));
            }
        }
        if (solver.objects.size() < MAX_OBJECT_COUNT && clock.getElapsedTime().asSeconds() >= SPAWN_DELAY) {
            clock.restart();
            auto& object = solver.addObject(sf::Vector2f(SPAWN_POSITION), RNGf::getRange(MIN_RADIUS, MAX_RADIUS));
            const float t = solver.time;
            const float angle = MAX_ANGLE * sin(t) + M_PI * 0.5f;
            solver.setObjectVelocity(object, SPAWN_SPEED * sf::Vector2f{cos(angle), sin(angle)});
            object.colour = getRainbow(t);
        }
        solver.updateThreaded();
        window.clear(sf::Color::White);
        renderer.render(solver);
        window.display();
    }

    return 0;
}

// cd /Users/vinzkakilala/Desktop/dev/multithreadedPhysicsSimulator/build
// cmake ..
// cmake --build .
// ./multithreadedPhysicsSimulator
