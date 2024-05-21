#pragma once

#include <SFML/Graphics.hpp>

#include "../maths/rng.hpp"
#include "../physics/solver.hpp"
#include "../renderer/renderer.hpp"
#include "../thread_pool/thread_pool.hpp"

sf::Color getRainbowColour(float t) {
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * M_PI);
    const float b = sin(t + 0.66f * 2.0f * M_PI);
    return {
        static_cast<uint8_t>(255.0f * r * r),
        static_cast<uint8_t>(255.0f * g * g),
        static_cast<uint8_t>(255.0f * b * b)
    };
}

void runSimulation(
    bool render_display,
    int32_t window_height,
    int32_t window_width,
    float max_object_count,
    float min_radius,
    float max_radius,
    float max_angle,
    bool speed_colouring,
    float spawn_delay,
    float spawn_speed,
    int32_t framerate_limit,
    int32_t thread_count,
    int32_t substeps,
    int8_t collision_resolver,
    sf::Vector2f spawn_position
) {
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Multithreaded Physics Engine", sf::Style::Default, settings);

    tp::ThreadPool thread_pool(thread_count);

    Solver solver(
        sf::Vector2f(window_width, window_height),
        substeps,
        max_radius,
        framerate_limit,
        speed_colouring,
        thread_pool
    );

    Renderer renderer{window};
    sf::Clock clock;
    RNG<float> rng;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            } else {
                solver.setAttractor(sf::Keyboard::isKeyPressed(sf::Keyboard::A));
                solver.setRepeller(sf::Keyboard::isKeyPressed(sf::Keyboard::R));
            }
        }
        if (solver.objects.size() < max_object_count && clock.getElapsedTime().asSeconds() >= spawn_delay) {
            clock.restart();
            VerletObject &object = solver.addObject(sf::Vector2f(spawn_position), rng.getRange(min_radius, max_radius));
            const float t = solver.time;
            object.colour = getRainbowColour(t);
            const float angle = max_angle * sin(t) + M_PI * 0.5f;
            solver.setObjectVelocity(object, spawn_speed * sf::Vector2f{cos(angle), sin(angle)});
        }
        switch (collision_resolver) {
            case 0: solver.updateThreaded(); break;
            case 1: solver.updateCellular(); break;
            case 2: solver.updateNaive(); break;
            default: solver.updateThreaded();
        }
        if (render_display) {
            window.clear(sf::Color::White);
            renderer.render(solver);
            window.display();
        }
    }
}
