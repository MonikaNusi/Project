#include "Bullet.h"

Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f dir, float speed, float lifeTime)
    : m_lifeTime(lifeTime)
{
    m_shape.setRadius(5.f);
    m_shape.setOrigin(5.f, 5.f);
    m_shape.setFillColor(sf::Color::Yellow);
    m_shape.setPosition(startPos);

    // normalize dir
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.001f)
        dir /= len;

    m_velocity = dir * speed;
}

void Bullet::update(sf::Time dt)
{
    if (!m_alive) return;

    float dtSec = dt.asSeconds();
    m_shape.move(m_velocity * dtSec);

    m_lifeTime -= dtSec;
    if (m_lifeTime <= 0.f)
        m_alive = false;
}

void Bullet::render(sf::RenderWindow& window) const
{
    if (m_alive)
        window.draw(m_shape);
}
