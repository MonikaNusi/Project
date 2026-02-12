#pragma once
#include <SFML/Graphics.hpp>

class Bullet
{
public:
    Bullet(sf::Vector2f startPos, sf::Vector2f dir, float speed = 600.f, float lifeTime = 1.5f);

    void update(sf::Time dt);
    void render(sf::RenderWindow& window) const;

    bool isAlive() const { return m_alive; }
    sf::FloatRect getBounds() const { return m_shape.getGlobalBounds(); }

    void kill() { m_alive = false; }

private:
    sf::CircleShape m_shape;
    sf::Vector2f m_velocity;
    float m_lifeTime;
    bool m_alive{ true };
};
