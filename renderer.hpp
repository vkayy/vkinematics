#pragma once

#include "solver.hpp"

constexpr float OUTLINE_THICKNESS = 2.0f;

class Renderer {
public:
    explicit
    Renderer(sf::RenderTarget &target) : target{target} {

    }
    void render(const Solver &solver) const {
        const sf::Vector3f constraint = solver.getConstraint();
        sf::CircleShape constraint_area{constraint.z};
        constraint_area.setOrigin(constraint.z, constraint.z);
        constraint_area.setFillColor(sf::Color::White);
        constraint_area.setPosition(constraint.x, constraint.y);
        constraint_area.setPointCount(8192);
        target.draw(constraint_area);

        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto& objects = solver.getObjects();
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
