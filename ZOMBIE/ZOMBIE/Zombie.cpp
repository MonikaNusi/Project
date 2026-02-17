#include "Zombie.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "Pathfinding.h"
#include <algorithm>

Zombie::Zombie(const sf::Texture& texture)
    : m_texture(&texture)
{
    m_sprite.setTexture(*m_texture);

    m_sprite.setOrigin(8.f, 12.f);


    m_sprite.setScale(m_baseScale, m_baseScale);

    m_health = m_maxHealth;

    //m_health = 100;

    m_cooldownTimer = m_cooldownDuration;
    m_hasDealtDamageThisAttack = false;

    m_walkDownTex.loadFromFile("ASSETS/IMAGES/ZombieDownWalk.png");
    m_walkDownTex.setSmooth(false);
    m_walkUpTex.loadFromFile("ASSETS/IMAGES/ZombieUpWalk.png");
    m_walkUpTex.setSmooth(false);
    m_walkSideTex.loadFromFile("ASSETS/IMAGES/ZombieSideWalk.png");
    m_walkSideTex.setSmooth(false);

    m_attackDownTex.loadFromFile("ASSETS/IMAGES/ZombieDownAttack.png");
    m_attackDownTex.setSmooth(false);
    m_attackUpTex.loadFromFile("ASSETS/IMAGES/ZombieUpAttack.png");
    m_attackUpTex.setSmooth(false);
    m_attackSideTex.loadFromFile("ASSETS/IMAGES/ZombieSideAttack.png");
    m_attackSideTex.setSmooth(false);


    m_healthBarBack.setFillColor(sf::Color(80, 0, 0));
    m_healthBarBack.setOutlineThickness(1.f);
    m_healthBarBack.setOutlineColor(sf::Color::Black);

    m_healthBarFront.setFillColor(sf::Color(200, 0, 0));

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
    if (m_health < 0) m_health = 0;
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
        m_hasDealtDamageThisAttack = false;

        m_anim = Anim::Attack;
        m_frameIndex = 0;
        m_frameTimer = 0.f;
    }
    else if (newState == State::Patrol || newState == State::Chase)
    {
        if (m_anim != Anim::Walk)
        {
            m_anim = Anim::Walk;
            m_frameIndex = 0;      
            m_frameTimer = 0.f;  
        }

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


    float attackEnter = m_attackRange;
    float attackExit  = m_attackRange * 0.9f;
   

    if (m_state != State::Attack)
    {
        if (m_cooldownTimer < m_cooldownDuration)
            m_cooldownTimer += ds;
    }

    if (m_state == State::Idle)
    {
        setState(State::Patrol);
    }

    if (m_state == State::Chase || m_state == State::Cooldown)
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
        else
        {
           
            m_moveTarget = playerPos;
        }
    }

    if (m_state != State::Attack)
    {
        if (dist <= attackEnter && m_cooldownTimer >= m_cooldownDuration)
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

        if (dist <= attackEnter && m_cooldownTimer >= m_cooldownDuration)
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

        sf::Vector2f dir = playerPos - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir /= len;
            m_velocity = dir * (m_speedChase * 1.2f);
        }
        else
        {
            m_velocity = { 0.f, 0.f };
        }

        if (m_attackTimer >= m_attackDuration)
        {
            m_cooldownTimer = 0.f;
            setState(State::Chase);
        }
    }
    else if (m_state == State::Cooldown)
    {
        m_cooldownTimer += ds;

        sf::Vector2f dir = m_moveTarget - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 2.f)
        {
            dir /= len;
            m_velocity = dir * (m_speedChase * 0.6f); // slower during cooldown
        }
        else
        {
            m_moveTarget = playerPos;
        }

        if (m_cooldownTimer >= m_cooldownDuration)
        {
            if (dist <= attackExit && m_cooldownTimer >= m_cooldownDuration) setState(State::Attack);
            else if (dist <= m_chaseRange) setState(State::Chase);
            else setState(State::Patrol);
        }
    }

    if (m_velocity != sf::Vector2f{ 0.f, 0.f })
    {
        updateFacing(m_velocity);
    }

    m_sprite.move(m_velocity * ds);

    animate(dt);


}

void Zombie::render(sf::RenderWindow& window)
{
    window.draw(m_sprite);

    // Draw health bar above head
    sf::FloatRect b = m_sprite.getGlobalBounds();

    // Width scaled to sprite width so healthbar fits nicely
    float barW = b.width * 0.6f;
    float barH = m_healthBarHeight;
    float x = b.left + (b.width - barW) * 0.5f;
    float y = b.top - barH - m_healthBarOffset;

    float healthPercent = 0.f;
    if (m_maxHealth > 0)
        healthPercent = std::max(0, m_health) / static_cast<float>(m_maxHealth);

    m_healthBarBack.setSize({ barW, barH });
    m_healthBarBack.setPosition(x, y);

    m_healthBarFront.setSize({ barW * healthPercent, barH });
    m_healthBarFront.setPosition(x, y);

    window.draw(m_healthBarBack);
    window.draw(m_healthBarFront);
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

int Zombie::tryDealDamage(const sf::FloatRect& playerHitbox)
{
    if (m_state == State::Attack && !m_hasDealtDamageThisAttack)
    {
        if (getHitbox().intersects(playerHitbox))
        {
            m_hasDealtDamageThisAttack = true;
            return 10;
        }
    }
    return 0;
}


void Zombie::reset(sf::Vector2f spawnPos)
{
    m_sprite.setPosition(spawnPos);
    m_velocity = { 0.f, 0.f };

    m_state = State::Idle;
    m_prevState = State::Idle;

    m_attackTimer = 0.f;
    m_cooldownTimer = m_cooldownDuration;
    m_patrolChangeTimer = 0.f;
    m_patrolDir = { 0.f, 0.f };

    m_health = m_maxHealth;
}

void Zombie::updateFacing(sf::Vector2f dir)
{
    if (std::abs(dir.x) > std::abs(dir.y))
        m_facing = Facing::Side;
    else if (dir.y < 0)
        m_facing = Facing::Up;
    else
        m_facing = Facing::Down;
}

void Zombie::animate(sf::Time dt)
{
    m_frameTimer += dt.asSeconds();
    if (m_frameTimer < m_frameTime)
        return;

    m_frameTimer = 0.f;
    m_frameIndex++;

    if (m_anim == Anim::Walk)
    {
        m_frameIndex %= 8;

        if (m_facing == Facing::Down)
        {
            m_sprite.setTexture(m_walkDownTex);
            m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 24 });
        }
        else if (m_facing == Facing::Up)
        {
            m_sprite.setTexture(m_walkUpTex);
            m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 24 });
        }
        else
        {
            m_sprite.setTexture(m_walkSideTex);
            m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 24 });
            float sign = (m_velocity.x < 0.f) ? -1.f : 1.f;
            m_sprite.setScale(sign * m_baseScale, m_baseScale);
        }
    }
    else // ATTACK
    {
        m_frameIndex = std::min(m_frameIndex + 1, 14);

        if (m_facing == Facing::Down)
        {
            m_sprite.setTexture(m_attackDownTex);
            m_sprite.setTextureRect({ m_frameIndex * 22, 0, 22, 30 });
            m_sprite.setScale(m_baseScale, m_baseScale);
        }
        else if (m_facing == Facing::Up)
        {
            m_sprite.setTexture(m_attackUpTex);
            m_sprite.setTextureRect({ m_frameIndex * 27, 0, 27, 24 });
            m_sprite.setScale(m_baseScale, m_baseScale);
        }
        else
        {
            m_sprite.setTexture(m_attackSideTex);
            m_sprite.setTextureRect({
            m_frameIndex * 30,
            0,
            30,
            23
                });
            float sign = (m_velocity.x < 0.f) ? -1.f : 1.f;
            m_sprite.setScale(sign * m_baseScale, m_baseScale);
        }
    }
}


