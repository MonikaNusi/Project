#pragma once
#include <SFML/System.hpp>
#include <vector>

// A* pathfinding on a tile grid
std::vector<sf::Vector2i> findPath(
    const std::vector<std::vector<int>>& tiles,
    sf::Vector2i start,
    sf::Vector2i goal
);