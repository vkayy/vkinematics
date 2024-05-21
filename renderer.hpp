#pragma once

#include "solver.hpp"

constexpr float OUTLINE_THICKNESS = 2.0f;

class Renderer {
public:
    explicit
    Renderer(sf::RenderTarget &target) : target{target} {

    }
    void render(const Solver &solver) const {
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto& objects = solver.objects;
        for (const auto& obj : objects) {
            circle.setPosition(obj.curr_position);
            circle.setScale(obj.radius, obj.radius);
            circle.setFillColor(obj.colour);
            circle.setOutlineColor(sf::Color::Black);
            circle.setOutlineThickness(-OUTLINE_THICKNESS / obj.radius);
            target.draw(circle);
        }
    }
private:
    sf::RenderTarget &target;
};
