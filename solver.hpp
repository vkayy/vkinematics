#pragma once

#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "verlet.hpp"
#include "collision-grid.hpp"

constexpr int SUBSTEPS = 8;
constexpr float GRAVITY_CONST = 1000.0f;
constexpr float RESPONSE_COEF = 0.1f;
constexpr float ATTRACTOR_STRENGTH = 2000.0f;
constexpr float REPELLOR_STRENGTH = 2000.0f;


struct Solver {
public:
    std::vector<VerletObject> objects;
    CollisionGrid grid;
    sf::Vector2f world_size;
    sf::Vector2f gravity = {0.0f, GRAVITY_CONST};


    sf::Vector2f constraint_center;
    float constraint_radius = 100.0f;

    bool attractor_active = false;
    bool repellor_active = false;
    bool speed_colouring = false;

    uint32_t substeps;
    float frame_dt = 0.0f;
    float time = 0.0f;

    Solver() = default;

    Solver(sf::Vector2f size)
        : grid{static_cast<int32_t>(size.x), static_cast<int32_t>(size.y)}
        , world_size{static_cast<float>(size.x), static_cast<float>(size.y)}
        , substeps{SUBSTEPS}
    {
        grid.clear();
    }

    VerletObject &addObject(sf::Vector2f position, float radius) {
        return objects.emplace_back(position, radius);
    }

    void update() {
        time += frame_dt;
        const float step_dt = getStepDt();
        for (int i=0; i<substeps; i++) {
            applyGravity();
            if (attractor_active) {
                applyAttractor();
            }
            if (repellor_active) {
                applyRepellor();
            }
            checkCollisionsNaive(step_dt);
            applyConstraint();
            updateObjects(step_dt);
        }
    }

    void setFrameRate(int framerate) {
        frame_dt = 1.0f / static_cast<float>(framerate);
    }

    void setSubsteps(int substeps) {
        this->substeps = substeps;
    }

    void setConstraint(sf::Vector2f position, float radius) {
        constraint_center = position;
        constraint_radius = radius;
    }

    void setAttractor(bool active) {
        attractor_active = active;
    }

    void setRepellor(bool active) {
        repellor_active = active;
    }

    void setSpeedColouring(bool active) {
        speed_colouring = active;
    }

    void setObjectVelocity(VerletObject &object, sf::Vector2f velocity) {
        object.setVelocity(velocity, getStepDt());
    }

    const std::vector<VerletObject> &getObjects() const {
        return objects;
    }

    sf::Vector3f getConstraint() const {
        return {constraint_center.x, constraint_center.y, constraint_radius};
    }

    uint64_t getObjectsCount() {
        return objects.size();
    }

    float getTime() {
        return time;
    }

    float getStepDt() {
        return frame_dt / static_cast<float>(substeps);
    }
private:
    void applyGravity() {
        for (auto &obj : objects) {
            obj.acceleration += gravity;
        }
    }

    void checkCollisionsNaive(float dt) {
        for (int i=0; i<getObjectsCount(); i++) {
            for (int j=i+1; j<getObjectsCount(); j++) {
                handleCollision(i, j);
            }
        }
    }

    void checkObjectCellCollisions(uint32_t object_id, const CollisionCell &cell) {
        for (uint32_t i=0; i<cell.object_count; i++) {
            handleCollision(object_id, cell.objects[i]);
        }
    }

    void checkCellCollisions(const CollisionCell &cell, uint32_t index) {
        std::vector<std::pair<uint32_t, uint32_t>> neighbours = {
            {0, 0}, {0, 1}, {0, -1},
            {1, 0}, {1, 1}, {1, -1},
            {-1, 0}, {-1, 1}, {-1, -1},
        };
        for (uint32_t i=0; i<cell.object_count; i++) {
            const uint32_t object_id = cell.objects[i];
            for (const auto &[dx, dy] : neighbours) {
                checkObjectCellCollisions(object_id, grid.data[index + grid.width * dy + dx]);
            }
        }
    }

    void handleCollision(uint32_t object_id1, uint32_t object_id2) {
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

    void applyAttractor() {
        for (auto &obj : objects) {
            const sf::Vector2f displacement = constraint_center - obj.curr_position;
            const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (distance > 0) {
                sf::Vector2f force = (displacement / distance) * ATTRACTOR_STRENGTH;
                obj.accelerate(force);
            }
        }
    }

    void applyRepellor() {
        for (auto &obj : objects) {
            const sf::Vector2f displacement = constraint_center - obj.curr_position;
            const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (distance > 0) {
                sf::Vector2f force = (displacement / distance) * REPELLOR_STRENGTH;
                obj.accelerate(-force);
            }
        }
    }

    void applyConstraint() {
        for (auto &obj : objects) {
            const sf::Vector2f displacement = constraint_center - obj.curr_position;
            const float distance = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (distance > (constraint_radius - obj.radius)) {
                const sf::Vector2f collision_factor = displacement / distance;
                obj.curr_position = constraint_center - collision_factor * (constraint_radius - obj.radius);
            }
        }
    }

    void addObjectsToGrid() {
        grid.clear();
        for (uint32_t i=0; i<objects.size(); i++) {
            VerletObject &object = objects[i];
            if (object.curr_position.x > 1.0f && object.curr_position.x < world_size.x - 1.0f &&
                object.curr_position.y > 1.0f && object.curr_position.y < world_size.y - 1.0f) {
                    grid.addObject(static_cast<int32_t>(object.curr_position.x), static_cast<int32_t>(object.curr_position.y), i);
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
};
