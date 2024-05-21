#pragma once

#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "verlet.hpp"
#include "collision-grid.hpp"

constexpr int DEFAULT_SUBSTEPS = 8;
constexpr float MARGIN_WIDTH = 2.0f;
constexpr float GRAVITY_CONST = 1000.0f;
constexpr float RESPONSE_COEF = 0.1f;
constexpr float ATTRACTOR_STRENGTH = 2000.0f;
constexpr float REPELLOR_STRENGTH = 2000.0f;


struct Solver {
public:
    sf::Vector2f gravity = {0.0f, -GRAVITY_CONST};
    std::vector<VerletObject> objects;
    sf::Vector2f simulation_size;
    CollisionGrid grid;

    sf::Vector2f center;
    float cell_size;

    bool attractor_active = false;
    bool repellor_active = false;
    bool speed_colouring = false;

    int32_t substeps;
    float frame_dt = 0.0f;
    float time = 0.0f;

    Solver() = default;
 
    Solver(sf::Vector2f size, int32_t substeps, float cell_size, int32_t framerate, bool speed_colouring)
        : grid{static_cast<int32_t>(size.x / cell_size), static_cast<int32_t>(size.y / cell_size)}
        , simulation_size{static_cast<float>(size.x), static_cast<float>(size.y)}
        , substeps{DEFAULT_SUBSTEPS}
        , cell_size{cell_size}
        , frame_dt{1.0f / static_cast<float>(framerate)}
        , speed_colouring{speed_colouring}
        , center{0.5f * simulation_size}
    {
        grid.clear();
    }

    VerletObject &addObject(sf::Vector2f position, float radius) {
        return objects.emplace_back(position, radius);
    }
    
    void updateNaive() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            if (attractor_active) {
                applyAttractor();
            }
            if (repellor_active) {
                applyRepellor();
            }
            solveCollisionsNaive();
            applyGravity();
            applyBorders();
            updateObjects(step_dt);
        }
    }

    void updateCellular() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            addObjectsToGrid();
            if (attractor_active) {
                applyAttractor();
            }
            if (repellor_active) {
                applyRepellor();
            }
            solveCollisionsCellular();
            applyGravity();
            applyBorders();
            updateObjects(step_dt);
        }
    }

    void setAttractor(bool active) {
        attractor_active = active;
    }

    void setRepellor(bool active) {
        repellor_active = active;
    }

    void setObjectVelocity(VerletObject &object, sf::Vector2f velocity) {
        object.setVelocity(velocity, getStepDt());
    }

    float getStepDt() {
        return frame_dt / static_cast<float>(substeps);
    }

private:
    void applyGravity() {
        for (auto &obj : objects) {
            obj.acceleration -= gravity;
        }
    }
    
    void applyAttractor() {
        for (auto &obj : objects) {
            const sf::Vector2f displacement = center - obj.curr_position;
            const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (distance > 0) {
                sf::Vector2f force = (displacement / distance) * ATTRACTOR_STRENGTH;
                obj.accelerate(force);
            }
        }
    }

    void applyRepellor() {
        for (auto &obj : objects) {
            const sf::Vector2f displacement = center - obj.curr_position;
            const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (distance > 0) {
                sf::Vector2f force = (displacement / distance) * REPELLOR_STRENGTH;
                obj.accelerate(-force);
            }
        }
    }


    void solveCollision(uint32_t object_id1, uint32_t object_id2) {
        VerletObject &object1 = objects[object_id1];
        VerletObject &object2 = objects[object_id2];
        const sf::Vector2f displacement = object1.curr_position - object2.curr_position;
        const float square_distance = displacement.x * displacement.x + displacement.y * displacement.y;
        const float min_distance = object1.radius + object2.radius;
        if (square_distance < min_distance * min_distance) {
            const float distance = sqrt(square_distance);
            const sf::Vector2f collision_factor = displacement / distance;
            const float collision_ratio1 = object2.radius / (object1.radius + object2.radius);
            const float collision_ratio2 = object1.radius / (object1.radius + object2.radius);
            const float delta = 0.5f * RESPONSE_COEF * (distance - min_distance);
            object1.curr_position -= collision_factor * (collision_ratio1 * delta);
            object2.curr_position += collision_factor * (collision_ratio2 * delta);
        }
    }

    void solveCollisionsNaive() {
        for (int i=0; i<objects.size(); i++) {
            for (int j=i+1; j<objects.size(); j++) {
                solveCollision(i, j);
            }
        }
    }

    void updateObjects(float dt) {
        for (auto &obj : objects) {
            obj.updatePosition(dt);
            if (speed_colouring) {
                obj.updateColour(dt);
            }
        }
    }

    // void updateObjectsCellular(float dt) {
    //     for (auto &cell : grid.data) {
    //         for (auto &object_id : cell.objects) {
    //             VerletObject &obj = objects[object_id];
    //             obj.updatePosition(dt);
    //             if (speed_colouring) {
    //                 obj.updateColour(dt);
    //             }
    //         }
    //     }
    // }

    void applyBorders() {
        for (auto &obj : objects) {
            const float margin = MARGIN_WIDTH + obj.radius;
            sf::Vector2f collision_factor = {0.0f, 0.0f};
            if (obj.curr_position.x > simulation_size.x - margin) {
                collision_factor += {(obj.curr_position.x - simulation_size.x + margin), 0.0f};
            } else if (obj.curr_position.x < margin) {
                collision_factor -= {(margin - obj.curr_position.x), 0.0f};
            }
            if (obj.curr_position.y > simulation_size.y - margin) {
                collision_factor += {0.0f, (obj.curr_position.y - simulation_size.y + margin)};
            } else if (obj.curr_position.y < margin) {
                collision_factor -= {0.0f, (margin - obj.curr_position.y)};
            }
            obj.curr_position -= 0.5f * collision_factor * RESPONSE_COEF;
        }
    }

    void solveObjectCellCollisions(uint32_t object_id, const CollisionCell &cell) {
        for (uint32_t i=0; i<cell.object_count; i++) {
            if (object_id != cell.objects[i]) {
                solveCollision(object_id, cell.objects[i]);
            }
        }
    }

    // void processCell(const CollisionCell &cell, uint32_t index) {
    //     for (uint32_t i=0; i<cell.object_count; i++) {
    //         const uint32_t object_id = cell.objects[i];
    //         // for (uint32_t nx=-1; nx<=1; nx++) {
    //         //     for (uint32_t ny=-1; ny<=1; ny++) {
    //         //         solveObjectCellCollisions(object_id, grid.data[index + nx * grid.height + ny]);
    //         //     }
    //         // }
    //         solveObjectCellCollisions(object_id, grid.data[index - 1]);
    //         solveObjectCellCollisions(object_id, grid.data[index]);
    //         solveObjectCellCollisions(object_id, grid.data[index + 1]);
    //         solveObjectCellCollisions(object_id, grid.data[index + grid.height - 1]);
    //         solveObjectCellCollisions(object_id, grid.data[index + grid.height]);
    //         solveObjectCellCollisions(object_id, grid.data[index + grid.height + 1]);
    //         solveObjectCellCollisions(object_id, grid.data[index - grid.height - 1]);
    //         solveObjectCellCollisions(object_id, grid.data[index - grid.height]);
    //         solveObjectCellCollisions(object_id, grid.data[index - grid.height + 1]);
    //     }
    // }

    void processCell(const CollisionCell &cell, uint32_t index) {
        const int32_t x = index / grid.height;
        const int32_t y = index % grid.height;

        for (const auto &object_id : cell.objects) {
            if (x > 0) {
                solveObjectCellCollisions(object_id, grid.data[index - grid.height]);
                if (y > 0) solveObjectCellCollisions(object_id, grid.data[index - grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.data[index - grid.height + 1]);
            }
            if (x < grid.width - 1) {
                solveObjectCellCollisions(object_id, grid.data[index + grid.height]);
                if (y > 0) solveObjectCellCollisions(object_id, grid.data[index + grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.data[index + grid.height + 1]);
            }
            if (y > 0) solveObjectCellCollisions(object_id, grid.data[index - 1]);
            if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.data[index + 1]);
            solveObjectCellCollisions(object_id, grid.data[index]);
        }
    }


    void solveCollisionsCellular() {
        for (uint32_t idx=0; idx<grid.data.size(); idx++) {
            processCell(grid.data[idx], idx);
        }
    }

    void addObjectsToGrid() {
        grid.clear();
        for (uint32_t idx=0; idx<objects.size(); idx++) { 
            VerletObject &object = objects[idx];
            if (object.curr_position.x > 1.0f && object.curr_position.x < simulation_size.x - 1.0f &&
                object.curr_position.y > 1.0f && object.curr_position.y < simulation_size.y - 1.0f) {
                grid.addObject(static_cast<int32_t>(object.curr_position.x / cell_size), static_cast<int32_t>(object.curr_position.y / cell_size), idx);
            }
        }
    }
};
