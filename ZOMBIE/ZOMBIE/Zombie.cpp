#include "Zombie.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

Zombie::Zombie()
{
    if (!m_texture.loadFromFile("ASSETS\\IMAGES\\zombie.png"))
    {
        std::cout << "Failed to load zombie texture\n";
    }
    m_sprite.setTexture(m_texture);

    m_sprite.setOrigin(
        m_sprite.getLocalBounds().width * 0.5f,
        m_sprite.getLocalBounds().height * 0.5f
    );

    m_sprite.setPosition(600.f, 500.f);

    m_sprite.setScale(1.f, 1.f);
}

void Zombie::setPosition(float x, float y)
{
    m_sprite.setPosition(x, y);
}

sf::Vector2f Zombie::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect Zombie::getBounds() const
{
    return m_sprite.getGlobalBounds();
}

float Zombie::distanceTo(sf::Vector2f p) const
{
    sf::Vector2f d = p - getPosition();
    return std::sqrt(d.x * d.x + d.y * d.y);
}

void Zombie::setState(State newState)
{
    if (newState == m_state) return;

    m_prevState = m_state;
    m_state = newState;

    std::cout << "[Zombie] State change to " << static_cast<int>(m_state) << "\n";

    if (m_state == State::Attack)
    {
        m_attackTimer = 0.f;
    }
    if (m_state == State::Cooldown)
    {
        m_cooldownTimer = 0.f;
    }
}

void Zombie::update(sf::Time dt, sf::Vector2f playerPos)
{
    float ds = dt.asSeconds();
    float dist = distanceTo(playerPos);

    if (m_state != State::Attack && m_state != State::Cooldown)
    {
        if (dist <= m_attackRange)
        {
            setState(State::Attack);
        }
        else if (dist <= m_chaseRange)
        {
            setState(State::Chase);
        }
        else
        {
            if (m_state == State::Idle) setState(State::Patrol);
        }
    }

    m_velocity = { 0.f, 0.f };

    if (m_state == State::Patrol)
    {
        m_patrolChangeTimer += ds;
        if (m_patrolChangeTimer >= m_patrolChangeInterval)
        {
            m_patrolChangeTimer = 0.f;

            int rx = (std::rand() % 3) - 1;
            int ry = (std::rand() % 3) - 1;

            sf::Vector2f dir((float)rx, (float)ry);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

            if (len > 0.001f)
            {
                dir /= len;
                m_velocity = dir * m_speedPatrol;
            }
        }
    }
    else if (m_state == State::Chase)
    {
        sf::Vector2f dir = playerPos - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir /= len;
            m_velocity = dir * m_speedChase;
        }
    }
    else if (m_state == State::Attack)
    {
        m_attackTimer += ds;

        if (m_attackTimer >= m_attackDuration)
        {
            setState(State::Cooldown);
        }
    }
    else if (m_state == State::Cooldown)
    {
        m_cooldownTimer += ds;

        if (m_cooldownTimer >= m_cooldownDuration)
        {
            if (dist <= m_attackRange) setState(State::Attack);
            else if (dist <= m_chaseRange) setState(State::Chase);
            else setState(State::Patrol);
        }
    }

    m_sprite.move(m_velocity * ds);
}

void Zombie::render(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}
