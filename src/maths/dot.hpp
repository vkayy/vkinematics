#include <SFML/Graphics.hpp>

float dot(sf::Vector2f v1, sf::Vector2f v2) {
    return v1.x * v2.x + v1.y * v2.y;
}
