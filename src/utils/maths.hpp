#pragma once

#include <random>

#include "../physics/verlet.hpp"

template <typename T>
struct RNG {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<T> dis;

    RNG()
        : rd{}
        , gen{rd()}
        , dis{0.0f, 1.0f}
    {}

    float get() {
        return dis(gen);
    }

    float getUnder(T max) {
        return get() * max;
    }

    float getRange(T min, T max) {
        return min + get() * (max - min);
    }

    float getRange(T width) {
        return getRange(-width * 0.5f, width * 0.5f);
    }
};

float calculatePolygonArea(std::vector<VerletObject*> &vertices) {
    float area = 0.0f;
    int32_t points = vertices.size();
    for (int32_t i=0; i<points; i++) {
        const sf::Vector2f &vertex1 = vertices[i]->curr_position;
        const sf::Vector2f &vertex2 = vertices[(i + 1) % points]->curr_position;
        area += vertex1.x * vertex2.y - vertex2.x * vertex1.y;
    }
    return std::abs(area) / 2.0f;
}
