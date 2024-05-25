#pragma once

#include <iostream>

#include <SFML/Graphics.hpp>

constexpr float DEFAULT_RADIUS = 10.0f;
constexpr float COLOUR_COEFFICIENT = 0.0015f;

struct VerletObject {
    sf::Vector2f curr_position = {0.0f, 0.0f};
    sf::Vector2f last_position = {0.0f, 0.0f};;
    sf::Vector2f acceleration = {0.0f, 0.0f};
    sf::Color colour = sf::Color::Red;
    float radius = DEFAULT_RADIUS;
    bool fixed;

    VerletObject() = default;
    VerletObject(sf::Vector2f pos, float radius, bool fixed)
        : curr_position{pos}
        , last_position{pos}
        , acceleration{0.0f, 0.0f}
        , radius{radius}
        , fixed{fixed}
    {}

    void updatePosition(float dt) {
        const sf::Vector2f displacement = curr_position - last_position;
        last_position = curr_position;
        curr_position = curr_position + displacement + acceleration * dt * dt;
        acceleration = {};
    }

    void updateColour(float dt) {
        const sf::Vector2f velocity = getVelocity(dt);
        const float colour_theta = COLOUR_COEFFICIENT * sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        const float r = sin(colour_theta);
        const float g = sin(colour_theta + 0.33f * 2.0f * M_PI);
        const float b = sin(colour_theta + 0.66f * 2.0f * M_PI);
        this->colour = {
            static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)
        };
    }

    void accelerate(sf::Vector2f a) {
        acceleration += a;
    }

    void addVelocity(sf::Vector2f v, float dt) {
        last_position -= v * dt;
    }

    void setVelocity(sf::Vector2f v, float dt) {
        last_position = curr_position - v * dt;
    }

    sf::Vector2f getVelocity(float dt) {
        return (curr_position - last_position) / dt;
    }
};

struct VerletConstraint {
    VerletObject &object_1;
    VerletObject &object_2;
    float target_distance;
    
    VerletConstraint(
        VerletObject &object_1,
        VerletObject &object_2,
        float target_distance
    )
    : object_1{object_1}
    , object_2{object_2}
    , target_distance{target_distance}
    {}

    void apply() {
        if (object_1.fixed && object_2.fixed) return;
        const sf::Vector2f axis = object_1.curr_position - object_2.curr_position;
        const float distance = sqrt(axis.x * axis.x + axis.y * axis.y) - (object_1.radius + object_2.radius);
        const sf::Vector2f normal = axis * distance / sqrt(axis.x * axis.x + axis.y * axis.y);
        const float delta = target_distance - distance;
        if (object_1.fixed && !object_2.fixed) {
            object_2.curr_position -= 0.02f * delta * normal;
        } else if (!object_1.fixed && object_2.fixed) {
            object_1.curr_position += 0.02f * delta * normal;
        } else {
            object_1.curr_position += 0.01f * delta * normal;      
            object_2.curr_position -= 0.01f * delta * normal;
        }
    }
};
