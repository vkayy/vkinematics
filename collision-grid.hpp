#pragma once

#include "grid.hpp"

constexpr uint8_t CELL_CAPACITY = 8;

struct CollisionCell {
    uint32_t object_count = 0;
    uint32_t objects[CELL_CAPACITY] = {};

    CollisionCell() = default;

    void addObject(uint32_t object_id) {
        objects[object_count] = object_id;
        object_count += object_count < (CELL_CAPACITY - 1);
    }

    void clear() {
        object_count = 0u;
    }

    void remove(uint32_t object_id) {
        for (uint32_t i=0; i<object_count; i++) {
            if (objects[i] == object_id) {
                objects[i] = objects[--object_count];
                return;
            }
        }
    }
};

struct CollisionGrid : Grid<CollisionCell> {
    CollisionGrid()
        : Grid<CollisionCell>()
    {}

    CollisionGrid(int32_t width, int32_t height)
        : Grid<CollisionCell>(width, height)
    {}

    void addObject(uint32_t x, uint32_t y, uint32_t object_id) {
        const uint32_t idx = x * height + y;
        data[idx].addObject(object_id);
    }

    void clear() {
        for (auto &cell : data) {
            cell.clear();
        }
    }
};
