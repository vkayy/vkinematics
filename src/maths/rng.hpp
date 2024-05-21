#pragma once

#include <random>

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
