#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "solver.hpp"
#include "renderer.hpp"
#include "utils/number_generator.hpp"

const sf::Vector2f SPAWN_POSITION = {500.0f, 200.0f};
constexpr float SPAWN_DELAY = 0.01f;
constexpr float SPAWN_SPEED = 10.0f;
constexpr float MIN_RADIUS = 5.0f;
constexpr float MAX_RADIUS = 20.0f;
constexpr float MAX_OBJECT_COUNT = 200;
constexpr float MAX_ANGLE = 1.0f;

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
    constexpr int32_t window_width = 1000;
    constexpr int32_t window_height = 1000;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 10;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Multithreaded Physics Simulator", sf::Style::Default, settings);
    const uint framerate_limit = 60;
    window.setFramerateLimit(framerate_limit);

    Solver solver;
    Renderer renderer{window};

    solver.setConstraint({static_cast<float>(window_width) * 0.5f, static_cast<float>(window_height) * 0.5f}, 450.0f);
    solver.setSubsteps(8);
    solver.setFrameRate(framerate_limit);
    solver.setSpeedColouring(true);

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
        if (solver.getObjectsCount() < MAX_OBJECT_COUNT && clock.getElapsedTime().asSeconds() >= SPAWN_DELAY) {
            clock.restart();
            auto& object = solver.addObject(sf::Vector2f(SPAWN_POSITION), RNGf::getRange(MIN_RADIUS, MAX_RADIUS));
            const float t = solver.getTime();
            const float angle = MAX_ANGLE * sin(t) + M_PI * 0.5f;
            solver.setObjectVelocity(object, SPAWN_SPEED * sf::Vector2f{cos(angle), sin(angle)});
            object.colour = getRainbow(t);
        }
        solver.update();
        window.clear(sf::Color::Black);
        renderer.render(solver);
        window.display();
    }

    return 0;
}

// cd /Users/vinzkakilala/Desktop/dev/multithreadedPhysicsSimulator/build
// cmake ..
// cmake --build .
// ./multithreadedPhysicsSimulator
