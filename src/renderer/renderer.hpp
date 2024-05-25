#pragma once

#include "../physics/solver.hpp"

constexpr float OUTLINE_THICKNESS = 2.0f;

class Renderer {
public:
    explicit
    Renderer(sf::RenderTarget &target)
        : target{target}
    {}
    void render(const Solver &solver) const {
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto &objects = solver.objects;
        for (const auto &object : objects) {
            circle.setPosition(object.curr_position);
            circle.setScale(object.radius, object.radius);
            circle.setFillColor(object.colour);
            circle.setOutlineColor(sf::Color::Black);
            circle.setOutlineThickness(-OUTLINE_THICKNESS / object.radius);
            target.draw(circle);
        }

        sf::Vertex line[2];
        const auto &constraints = solver.constraints;
        for (const auto &constraint : constraints) {
            line[0].position = constraint.object_1.curr_position;
            line[1].position = constraint.object_2.curr_position;
            line[0].color = sf::Color::Black;
            line[1].color = sf::Color::Black;
            target.draw(line, 2, sf::Lines);
        }
    }
private:
    sf::RenderTarget &target;
};
