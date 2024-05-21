#pragma once

#include <vector>

template<typename T>
struct UniformGrid {
    int32_t width, height;
    std::vector<T> cells;

    UniformGrid()
        : width(0)
        , height(0)
    {}

    UniformGrid(int32_t width, int32_t height)
        : width{width}
        , height{height}
    {
        cells.resize(width * height);
    }

    template<typename Vector2Type>
    void set(const Vector2Type &coords, const T& object) {
        set(coords.x, coords.y, object);
    }

    void set(int32_t x, int32_t y, const T& object) {
        cells[width * y + x] = object;
    }
};
