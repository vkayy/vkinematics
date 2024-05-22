#pragma once

#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "verlet.hpp"
#include "uniform-collision-grid.hpp"
#include "../thread_pool/thread_pool.hpp"

constexpr int DEFAULT_SUBSTEPS = 8;
constexpr float MARGIN_WIDTH = 2.0f;
constexpr float GRAVITY_CONST = 1000.0f;
constexpr float RESPONSE_COEF = 0.1f;
constexpr float ATTRACTOR_STRENGTH = 2000.0f;
constexpr float REPELLER_STRENGTH = 2000.0f;


struct Solver {
public:
    sf::Vector2f gravity = {0.0f, -GRAVITY_CONST};
    std::vector<VerletObject> objects;
    sf::Vector2f simulation_size;
    UniformCollisionGrid grid;

    sf::Vector2f center;
    float cell_size;

    bool attractor_active = false;
    bool repeller_active = false;
    bool speed_colouring = false;

    int32_t substeps;
    float frame_dt = 0.0f;
    float time = 0.0f;

    tp::ThreadPool &thread_pool;
 
    Solver(
        sf::Vector2f size,
        int32_t substeps,
        float cell_size,
        int32_t framerate,
        bool speed_colouring,
        tp::ThreadPool &thread_pool,
        bool gravity_on
    )
        : grid{static_cast<int32_t>(size.x / cell_size), static_cast<int32_t>(size.y / cell_size)}
        , simulation_size{static_cast<float>(size.x), static_cast<float>(size.y)}
        , substeps{DEFAULT_SUBSTEPS}
        , cell_size{cell_size}
        , frame_dt{1.0f / static_cast<float>(framerate)}
        , speed_colouring{speed_colouring}
        , center{0.5f * simulation_size}
        , thread_pool{thread_pool}
        , gravity{sf::Vector2f(0.0f, gravity_on ? -GRAVITY_CONST : 0.0f)}
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
            solveCollisionsNaive();
            updateObjects(step_dt);
        }
    }

    void updateCellular() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            addObjectsToGrid();
            solveCollisionsCellular();
            updateObjects(step_dt);
        }
    }

    void updateThreaded() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            addObjectsToGrid();
            solveCollisionsThreaded();
            updateObjectsThreaded(step_dt);
        }
    }

    void setAttractor(bool active) {
        attractor_active = active;
    }

    void setRepeller(bool active) {
        repeller_active = active;
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

    void applyAttractor(VerletObject &object) {
        const sf::Vector2f displacement = center - object.curr_position;
        const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
        if (distance > 0) {
            sf::Vector2f force = (displacement / distance) * ATTRACTOR_STRENGTH;
            object.accelerate(force);
        }
    }

    void applyRepeller(VerletObject &object) {
        const sf::Vector2f displacement = center - object.curr_position;
        const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
        if (distance > 0) {
            sf::Vector2f force = (displacement / distance) * REPELLER_STRENGTH;
            object.accelerate(-force);
        }
    }

    void applyBorders(VerletObject &object) {
        const float margin = MARGIN_WIDTH + object.radius;
        sf::Vector2f collision_factor = {0.0f, 0.0f};
        if (object.curr_position.x > simulation_size.x - margin) {
            collision_factor += {(object.curr_position.x - simulation_size.x + margin), 0.0f};
        } else if (object.curr_position.x < margin) {
            collision_factor -= {(margin - object.curr_position.x), 0.0f};
        }
        if (object.curr_position.y > simulation_size.y - margin) {
            collision_factor += {0.0f, (object.curr_position.y - simulation_size.y + margin)};
        } else if (object.curr_position.y < margin) {
            collision_factor -= {0.0f, (margin - object.curr_position.y)};
        }
        object.curr_position -= 0.5f * collision_factor * RESPONSE_COEF;
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

    void solveObjectCellCollisions(uint32_t object_id, const CollisionCell &cell) {
        for (uint32_t i=0; i<cell.object_count; i++) {
            if (object_id != cell.objects[i]) {
                solveCollision(object_id, cell.objects[i]);
            }
        }
    }

    void processCell(const CollisionCell &cell, uint32_t index) {
        const int32_t x = index / grid.height;
        const int32_t y = index % grid.height;
        for (const auto &object_id : cell.objects) {
            solveObjectCellCollisions(object_id, grid.cells[index]);
            if (y > 1) solveObjectCellCollisions(object_id, grid.cells[index - 1]);
            if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index + 1]);
            if (x > 1) {
                solveObjectCellCollisions(object_id, grid.cells[index - grid.height]);
                if (y > 1) solveObjectCellCollisions(object_id, grid.cells[index - grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index - grid.height + 1]);
            }
            if (x < grid.width - 1) {
                solveObjectCellCollisions(object_id, grid.cells[index + grid.height]);
                if (y > 1) solveObjectCellCollisions(object_id, grid.cells[index + grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index + grid.height + 1]);
            }
        }
    }


    void solveCollisionsCellular() {
        for (uint32_t idx=0; idx<grid.cells.size(); idx++) {
            if (grid.cells[idx].object_count > 0 &&
                idx / grid.height > 0 && idx / grid.height < grid.width - 1 &&
                idx % grid.height > 0 && idx % grid.height < grid.height - 1) {
                processCell(grid.cells[idx], idx);
            }
        }
    }

    void updateObjects(float dt) {
        for (auto &object : objects) {
            object.acceleration -= gravity;
            if (attractor_active) {
                applyAttractor(object);
            }
            if (repeller_active) {
                applyRepeller(object);
            }
            object.updatePosition(dt);
            if (speed_colouring) {
                object.updateColour(dt);
            }
            applyBorders(object);
        }
    }

    void updateObjectsThreaded(float dt) {
        thread_pool.dispatch(objects.size(), [&](uint32_t start, uint32_t end) {
            for (uint32_t idx=start; idx<end; idx++) {
                VerletObject &object = objects[idx];
                object.acceleration -= gravity;
                if (attractor_active) {
                    applyAttractor(object);
                }
                if (repeller_active) {
                    applyRepeller(object);
                }
                object.updatePosition(dt);
                if (speed_colouring) {
                    object.updateColour(dt);
                }
                applyBorders(object);
            }
        });
    }

    void solvePartitionThreaded(uint32_t start, uint32_t end) {
        for (uint32_t idx=start; idx<end; idx++) {
            if (grid.cells[idx].object_count > 0 &&
                idx / grid.height > 0 && idx / grid.height < grid.width - 1 &&
                idx % grid.height > 0 && idx % grid.height < grid.height - 1) {
                processCell(grid.cells[idx], idx);
            }
        }
    }

    void solveCollisionsThreaded() {
        const uint32_t thread_count = thread_pool.thread_count;
        const uint32_t partition_count = thread_count * 2;
        const uint32_t partition_size = (grid.width / partition_count) * grid.height;
        const uint32_t last_cell = 2 * thread_count * partition_size;

        for (uint32_t idx=0; idx<thread_count; idx++) {
            thread_pool.enqueueTask([this, idx, partition_size]{
                uint32_t const start = 2 * idx * partition_size;
                uint32_t const end = start + partition_size;
                solvePartitionThreaded(start, end);
            });
        }
        if (last_cell < grid.cells.size()) {
            thread_pool.enqueueTask([this, last_cell]{
                solvePartitionThreaded(last_cell, grid.cells.size());
            });
        }
        for (uint32_t idx=0; idx<thread_count; idx++) {
            thread_pool.enqueueTask([this, idx, partition_size]{
                uint32_t const start = (2 * idx + 1) * partition_size;
                uint32_t const end = start + partition_size;
                solvePartitionThreaded(start, end);
            });
        }
        thread_pool.completeAllTasks();
    }
};
