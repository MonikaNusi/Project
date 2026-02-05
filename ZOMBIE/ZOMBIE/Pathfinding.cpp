#include "Pathfinding.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

struct PQNode
{
    sf::Vector2i pos;
    float priority;
};

struct CompareNode
{
    bool operator()(const PQNode& a, const PQNode& b) const
    {
        return a.priority > b.priority;
    }
};

static float heuristic(sf::Vector2i a, sf::Vector2i b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<sf::Vector2i> findPath(
    const std::vector<std::vector<int>>& tiles,
    sf::Vector2i start,
    sf::Vector2i goal)
{
    std::vector<sf::Vector2i> path;

    const int height = static_cast<int>(tiles.size());
    const int width = static_cast<int>(tiles[0].size());

    auto key = [&](sf::Vector2i p)
    {
        return p.y * width + p.x;
    };

    std::priority_queue<PQNode, std::vector<PQNode>, CompareNode> open;
    std::unordered_map<int, sf::Vector2i> cameFrom;
    std::unordered_map<int, float> costSoFar;

    open.push({ start, 0.f });
    costSoFar[key(start)] = 0.f;

    const sf::Vector2i directions[4] =
    {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };

    while (!open.empty())
    {
        sf::Vector2i current = open.top().pos;
        open.pop();

        if (current == goal)
            break;

        for (const auto& d : directions)
        {
            sf::Vector2i next = current + d;

            // Bounds check
            if (next.x < 0 || next.y < 0 ||
                next.x >= width || next.y >= height)
                continue;

            // Wall check
            if (tiles[next.y][next.x] == 1)
                continue;

            float newCost = costSoFar[key(current)] + 1.f;
            int nextKey = key(next);

            if (!costSoFar.count(nextKey) || newCost < costSoFar[nextKey])
            {
                costSoFar[nextKey] = newCost;
                float priority = newCost + heuristic(next, goal);
                open.push({ next, priority });
                cameFrom[nextKey] = current;
            }
        }
    }

    // Reconstruct path
    if (!cameFrom.count(key(goal)))
        return path; // no path found

    sf::Vector2i current = goal;
    while (current != start)
    {
        path.push_back(current);
        current = cameFrom[key(current)];
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());

    return path;
}
