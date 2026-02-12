#include "Zombie.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "Pathfinding.h"

Zombie::Zombie(const sf::Texture& texture)
    : m_texture(&texture)
{
    m_sprite.setTexture(*m_texture);

    m_sprite.setOrigin(
        m_sprite.getLocalBounds().width * 0.5f,
        m_sprite.getLocalBounds().height * 0.5f
    );

 
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

void Zombie::setFrozen(bool frozen)
{
    m_frozen = frozen;
}

void Zombie::takeDamage(int dmg)
{
    m_health -= dmg;
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

void Zombie::update(sf::Time dt, sf::Vector2f playerPos, const std::vector<std::vector<int>>& tiles,
    float tileW,
    float tileH)
{
    if (m_frozen)
        return;

    float ds = dt.asSeconds();
    float dist = distanceTo(playerPos);

    if (m_state == State::Idle)
    {
        setState(State::Patrol);
    }

    if (m_state == State::Chase)
    {
        sf::Vector2i zombieTile(
            static_cast<int>(getPosition().x / tileW),
            static_cast<int>(getPosition().y / tileH)
        );

        sf::Vector2i playerTile(
            static_cast<int>(playerPos.x / tileW),
            static_cast<int>(playerPos.y / tileH)
        );

        auto path = findPath(tiles, zombieTile, playerTile);

        if (path.size() > 1)
        {
            sf::Vector2i next = path[1];

            m_moveTarget = {
                next.x * tileW + tileW * 0.5f,
                next.y * tileH + tileH * 0.5f
            };
        }
    }

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
        if (m_patrolChangeTimer >= m_patrolChangeInterval || m_patrolDir == sf::Vector2f{ 0.f, 0.f })
        {
            m_patrolChangeTimer = 0.f;

            int rx = (std::rand() % 3) - 1;
            int ry = (std::rand() % 3) - 1;

            m_patrolDir = sf::Vector2f((float)rx, (float)ry);
            float len = std::sqrt(m_patrolDir.x * m_patrolDir.x + m_patrolDir.y * m_patrolDir.y);

            if (len > 0.001f)
                m_patrolDir /= len;
        }


        m_velocity = m_patrolDir * m_speedPatrol;
    }

    else if (m_state == State::Chase)
    {
        if (dist <= m_attackRange)
        {
            setState(State::Attack);
            return;
        }

        sf::Vector2f dir = m_moveTarget - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (len > 2.f)
        {
            dir /= len;
            m_velocity = dir * m_speedChase;
        }
        else
        {
            m_moveTarget = playerPos;
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

sf::FloatRect Zombie::getHitbox() const
{
    sf::FloatRect b = m_sprite.getGlobalBounds();

    float w = b.width * 0.35f;
    float h = b.height * 0.25f;

    return sf::FloatRect(
        b.left + (b.width - w) * 0.5f,
        b.top + b.height - h,
        w,
        h
    );
}

void Zombie::reset(sf::Vector2f spawnPos)
{
    m_sprite.setPosition(spawnPos);
    m_velocity = { 0.f, 0.f };

    m_state = State::Idle;
    m_prevState = State::Idle;

    m_attackTimer = 0.f;
    m_cooldownTimer = 0.f;
    m_patrolChangeTimer = 0.f;
    m_patrolDir = { 0.f, 0.f };
}

