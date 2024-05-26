#pragma once

#include <vector>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "verlet.hpp"
#include "uniform-collision-grid.hpp"
#include "../utils/maths.hpp"
#include "../thread_pool/thread_pool.hpp"

constexpr int DEFAULT_SUBSTEPS = 8;
constexpr int JAKOBSEN_ITERATIONS = 10;
constexpr float MARGIN_WIDTH = 2.0f;
constexpr float GRAVITY_CONST = 1000.0f;
constexpr float RESPONSE_COEF = 0.5f;
constexpr float ATTRACTOR_STRENGTH = 2000.0f;
constexpr float REPELLER_STRENGTH = 2000.0f;

struct Solver {
    Solver(
        sf::Vector2f size,
        int32_t substeps,
        float cell_size,
        int32_t max_object_count,
        int32_t framerate,
        bool speed_colouring,
        tp::ThreadPool &thread_pool,
        bool gravity_on
    )
        : grid{static_cast<int32_t>(size.x / cell_size + 1), static_cast<int32_t>(size.y / cell_size + 1)}
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
        objects.reserve(max_object_count);
        constraints.reserve(max_object_count);
        springs.reserve(max_object_count);
    }
public:
    std::vector<VerletObject> objects;
    std::vector<VerletConstraint> constraints;
    std::vector<VerletSpring> springs;
    std::vector<VerletBlob> blobs;
    int32_t body_count = 0;
    std::unordered_map<int32_t, int32_t> body;
    float time = 0.0f;

    VerletObject &addObject(sf::Vector2f position, float radius, bool fixed = false) {
        return objects.emplace_back(position, radius, fixed);
    }

    VerletConstraint &addConstraint(
        VerletObject &object1,
        VerletObject &object2,
        float target_distance
    ) {
        return constraints.emplace_back(object1, object2, target_distance);
    }

    VerletSpring &addSpring(
        VerletObject &object1,
        VerletObject &object2,
        float target_distance
    ) {
        return springs.emplace_back(object1, object2, target_distance);
    }

    VerletBlob &addBlob(
        std::vector<VerletObject*> vertices,
        std::vector<VerletConstraint*> segments,
        float radius
    ) {
        return blobs.emplace_back(vertices, segments, radius);
    }
    
    void updateNaive() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            solveCollisionsNaive();
            updateConstraints();
            updateSprings();
            updateBlobs();
            updateObjects(step_dt);
        }
    }

    void updateCellular() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            addObjectsToGrid();
            solveCollisionsCellular();
            updateConstraints();
            updateSprings();
            updateBlobs();
            updateObjects(step_dt);
        }
    }

    void updateThreaded() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            addObjectsToGrid();
            solveCollisionsThreaded();
            updateConstraints();
            updateSprings();
            updateBlobs();
            updateObjectsThreaded(step_dt);
        }
    }

    void setAttractor(bool active) {
        attractor_active = active;
    }

    void setRepeller(bool active) {
        repeller_active = active;
    }

    void setSpeedUp(bool active) {
        speedup_active = active;
    }

    void setSlowDown(bool active) {
        slowdown_active = active;
    }

    void setSlomo(bool active) {
        slomo_active = active;
    }

    void setObjectVelocity(VerletObject &object, sf::Vector2f velocity) {
        object.setVelocity(velocity, getStepDt());
    }

    float getStepDt() {
        return frame_dt / static_cast<float>(substeps);
    }

private:
    sf::Vector2f gravity = {0.0f, -GRAVITY_CONST};
    sf::Vector2f simulation_size;
    UniformCollisionGrid grid;
    sf::Vector2f center;
    float cell_size;
    bool attractor_active = false;
    bool repeller_active = false;
    bool speedup_active = false;
    bool slowdown_active = false;
    bool slomo_active = false;
    bool speed_colouring = false;
    int32_t substeps;
    float frame_dt = 0.0f;
    tp::ThreadPool &thread_pool;


    void applyGravity() {
        for (auto &obj : objects) {
            obj.acceleration -= gravity;
        }
    }

    void applyAttractor(VerletObject &object) {
        const sf::Vector2f displacement = center - object.curr_position;
        const float square_distance = displacement.x * displacement.x + displacement.y * displacement.y;
        if (square_distance > 0) {
            const float distance = sqrt(square_distance);
            sf::Vector2f force = (displacement / distance) * ATTRACTOR_STRENGTH;
            object.accelerate(force);
        }
    }

    void applyRepeller(VerletObject &object) {
        const sf::Vector2f displacement = center - object.curr_position;
        const float square_distance = displacement.x * displacement.x + displacement.y * displacement.y;
        if (square_distance > 0) {
            const float distance = sqrt(square_distance);
            sf::Vector2f force = (displacement / distance) * ATTRACTOR_STRENGTH;
            object.accelerate(-force);
        }
    }

    void applySpeedUp(VerletObject &object) {
        const float step_dt = getStepDt();
        const sf::Vector2f velocity = object.getVelocity(step_dt);
        object.addVelocity(0.001f * velocity, step_dt);
    }

    void applySlowDown(VerletObject &object) {
        const float step_dt = getStepDt();
        const sf::Vector2f velocity = object.getVelocity(step_dt);
        object.setVelocity(0.999f * velocity, step_dt);
    }

    void applySlomo(VerletObject &object) {
        const float step_dt = getStepDt();
        const sf::Vector2f velocity = object.getVelocity(step_dt);
        object.setVelocity(-velocity, step_dt);
    }

    void applyBorders(VerletObject &object) {
        const float margin = MARGIN_WIDTH + object.radius;
        sf::Vector2f collision_normal = {0.0f, 0.0f};
        if (object.curr_position.x > simulation_size.x - margin) {
            collision_normal += {(object.curr_position.x - simulation_size.x + margin), 0.0f};
        } else if (object.curr_position.x < margin) {
            collision_normal -= {(margin - object.curr_position.x), 0.0f};
        }
        if (object.curr_position.y > simulation_size.y - margin) {
            collision_normal += {0.0f, (object.curr_position.y - simulation_size.y + margin)};
        } else if (object.curr_position.y < margin) {
            collision_normal -= {0.0f, (margin - object.curr_position.y)};
        }
        object.curr_position -= 0.2f * collision_normal * RESPONSE_COEF;
    }

    void addObjectsToGrid() {
        grid.clear();
        for (uint32_t idx=0; idx<objects.size(); idx++) { 
            VerletObject &object = objects[idx];
            if (!object.radius) continue;
            if (object.curr_position.x > 1.0f && object.curr_position.x < simulation_size.x - 1.0f &&
                object.curr_position.y > 1.0f && object.curr_position.y < simulation_size.y - 1.0f) {
                grid.addObject(
                    static_cast<int32_t>(object.curr_position.x / cell_size),
                    static_cast<int32_t>(object.curr_position.y / cell_size),
                    idx
                );
            }
        }
    }

    void solveCollision(int32_t object_id1, int32_t object_id2) {
        if (body.count(object_id1) && body.count(object_id2)) {
            if (body[object_id1] == body[object_id2]) return;
        }
        VerletObject &object1 = objects[object_id1];
        VerletObject &object2 = objects[object_id2];
        if (object1.fixed && object2.fixed) return;
        const sf::Vector2f displacement = object1.curr_position - object2.curr_position;
        const float square_distance = displacement.x * displacement.x + displacement.y * displacement.y;
        const float min_distance = object1.radius + object2.radius;
        if (square_distance < min_distance * min_distance) {
            const float radius1 = body.count(object_id1) ? 20.0f : object1.radius;
            const float radius2 = body.count(object_id2) ? 20.0f : object2.radius;
            const float mass_proportion1 = radius1 * radius1 * radius1;
            const float mass_proportion2 = radius2 * radius2 * radius2;
            const float total_mass_proportion = mass_proportion1 + mass_proportion2;
            const float distance = sqrt(square_distance);
            const sf::Vector2f collision_normal = displacement / distance;
            const float collision_ratio1 = mass_proportion2 / total_mass_proportion;
            const float collision_ratio2 = mass_proportion1 / total_mass_proportion;
            const float delta = RESPONSE_COEF * (distance - min_distance);
            if (!object1.fixed && !object2.fixed) {
                object1.curr_position -= 0.5f * collision_normal * (collision_ratio1 * delta);
                object2.curr_position += 0.5f * collision_normal * (collision_ratio2 * delta);
            } else if (object1.fixed && !object2.fixed) {
                object2.curr_position += collision_normal * (collision_ratio1 * delta);
            } else {
                object1.curr_position -= collision_normal * (collision_ratio2 * delta);
            }
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
        for (int32_t i=0; i<cell.object_count; i++) {
            if (object_id != cell.objects[i]) {
                solveCollision(object_id, cell.objects[i]);
            }
        }
    }

    void processCell(const CollisionCell &cell, int32_t index) {
        const int32_t x = index / grid.height;
        const int32_t y = index % grid.height;
        for (const auto &object_id : cell.objects) {
            if (x > 0) {
                solveObjectCellCollisions(object_id, grid.cells[index - grid.height]);
                if (y > 0) solveObjectCellCollisions(object_id, grid.cells[index - grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index - grid.height + 1]);
            }
            if (x < grid.width - 1) {
                solveObjectCellCollisions(object_id, grid.cells[index + grid.height]);
                if (y > 0) solveObjectCellCollisions(object_id, grid.cells[index + grid.height - 1]);
                if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index + grid.height + 1]);
            }
            if (y > 0) solveObjectCellCollisions(object_id, grid.cells[index - 1]);
            if (y < grid.height - 1) solveObjectCellCollisions(object_id, grid.cells[index + 1]);
            solveObjectCellCollisions(object_id, grid.cells[index]);
        }
    }


    void solveCollisionsCellular() {
        for (uint32_t idx=0; idx<grid.cells.size(); idx++) {
            if (grid.cells[idx].object_count > 0) {
                processCell(grid.cells[idx], idx);
            }
        }
    }

    void updateObject(VerletObject &object, float dt) {
        if (!object.radius && !object.fixed) {
            object.acceleration -= gravity;
        }
        if (!object.fixed) {
            object.acceleration -= gravity;
            if (attractor_active) {
                applyAttractor(object);
            }
            if (repeller_active) {
                applyRepeller(object);
            }
            if (speedup_active) {
                applySpeedUp(object);
            }
            if (slowdown_active) {
                applySlowDown(object);
            }
            if (slomo_active) {
                applySlomo(object);
            }
        }
        object.updatePosition(dt);
        if (speed_colouring) {
            object.updateColour(dt);
        }
        applyBorders(object);
    }

    void updateObjects(float dt) {
        for (auto &object : objects) {
            updateObject(object, dt);
        }
    }

    void updateConstraints() {
        if (constraints.empty()) return;
        for (int i=0; i<JAKOBSEN_ITERATIONS; i++) {
            for (auto &constraint : constraints) {
                constraint.apply();
            }
        }
    }

    void updateSprings() {
        if (springs.empty()) return;
        for (int i=0; i<JAKOBSEN_ITERATIONS; i++) {
            for (auto &spring : springs) {
                spring.apply();
            }
        }
    }

    void updateBlobs() {
        if (blobs.empty()) return;
        for (int i=0; i<JAKOBSEN_ITERATIONS; i++) {
            for (auto &blob : blobs) {
                blob.apply();
            }
        }
    }

    void updateObjectsCellular(float dt) {
        for (auto &cell : grid.cells) {
            if (cell.object_count > 0) {
                for (auto &object_id : cell.objects) {
                    updateObject(objects[object_id], dt);
                }
            }
        }
    }

    void updateObjectsThreaded(float dt) {
        thread_pool.dispatch(objects.size(), [&](uint32_t start, uint32_t end) {
            for (uint32_t idx=start; idx<end; idx++) {
                updateObject(objects[idx], dt);
            }
        });
    }

    void solvePartitionThreaded(uint32_t start, uint32_t end) {
        for (uint32_t idx=start; idx<end; idx++) {
            if (grid.cells[idx].object_count > 0) {
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
        for (uint32_t idx=0; idx<thread_count; idx++) {
            thread_pool.enqueueTask([this, idx, partition_size]{
                uint32_t const start = (2 * idx + 1) * partition_size;
                uint32_t const end = start + partition_size;
                solvePartitionThreaded(start, end);
            });
        }
        if (last_cell < grid.cells.size()) {
            thread_pool.enqueueTask([this, last_cell]{
                solvePartitionThreaded(last_cell, grid.cells.size());
            });
        }
        thread_pool.completeAllTasks();
    }
};
