#pragma once

#include <vector>

template<typename T>
struct Grid {
    int32_t width, height;
    std::vector<T> data;

    Grid()
        : width(0)
        , height(0)
    {}

    Grid(int32_t width, int32_t height)
        : width{width}
        , height{height}
    {
        data.resize(width * height);
    }

    template<typename Vector2Type>
    bool checkCoords(const Vector2Type &coords) const {
        return checkCoords(static_cast<int32_t>(coords.x), static_cast<int32_t>(coords.y));
    }

    bool checkCoords(int32_t x, int32_t y) {
        return x > 0 && x < (width - 1) && y > 0 && y < (height - 1);
    }

    const T &get(int32_t x, int32_t y) const {
        return data[width * y + x];
    }

    template<typename Vector2Type>
    const T &get(const Vector2Type &coords) const {
        return get(static_cast<int32_t>(coords.x), static_cast<int32_t>(coords.y));
    }

    template<typename Vector2Type>
    void set(const Vector2Type &coords, const T& object) {
        set(coords.x, coords.y, object);
    }

    void set(int32_t x, int32_t y, const T& object) {
        data[width * y + x] = object;
    }
};
