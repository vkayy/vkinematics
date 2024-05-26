#pragma once

#include <iostream>

#include "../physics/solver.hpp"

constexpr float OUTLINE_THICKNESS = 0.0f;

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
        int32_t object_id = 0;
        const auto &objects = solver.objects;
        for (const auto &object : objects) {
            if (object.hidden) continue;
            circle.setPosition(object.curr_position);
            circle.setScale(object.radius, object.radius);
            circle.setFillColor(object.colour);
            circle.setOutlineColor(sf::Color::Black);
            circle.setOutlineThickness(-OUTLINE_THICKNESS / object.radius);
            target.draw(circle);
            object_id++;
        }

        sf::Vertex constraint_line[2];
        const auto &constraints = solver.constraints;
        for (const auto &constraint : constraints) {
            if (constraint.in_body) continue;
            constraint_line[0].position = constraint.object_1.curr_position;
            constraint_line[1].position = constraint.object_2.curr_position;
            constraint_line[0].color = sf::Color::Black;
            constraint_line[1].color = sf::Color::Black;
            target.draw(constraint_line, 2, sf::Lines);
        }

        sf::Vertex spring_line[2];
        const auto &springs = solver.springs;
        for (const auto &spring : springs) {
            if (spring.in_body) continue;
            spring_line[0].position = spring.object_1.curr_position;
            spring_line[1].position = spring.object_2.curr_position;
            spring_line[0].color = sf::Color::Black;
            spring_line[1].color = sf::Color::Black;
            target.draw(spring_line, 2, sf::Lines);
        }

        // sf::ConvexShape polygon;
        // const auto &blobs = solver.blobs;
        // for (const auto &blob : blobs) {
        //     polygon.setPointCount(blob.points);
        //     for (int32_t i=0; i<blob.points; i++) {
        //         polygon.setPoint(i, blob.vertices[i]->curr_position);
        //     }
        //     polygon.setOutlineColor(sf::Color::Black);
        //     polygon.setOutlineThickness(blob.vertices[0]->radius);
        //     polygon.setFillColor(blob.vertices[0]->colour);
        //     target.draw(polygon);
        // }


        const auto &blobs = solver.blobs;
        for (const auto &blob : blobs) {
            sf::Vertex polygon[blob.points];
            for (int32_t i=0; i<blob.points; i++) {
                polygon[i].position = blob.vertices[i]->curr_position;
                polygon[i].color = blob.vertices[i]->colour;
            }
            target.draw(polygon, blob.points, sf::TriangleFan);
        }
    }
private:
    sf::RenderTarget &target;
};
