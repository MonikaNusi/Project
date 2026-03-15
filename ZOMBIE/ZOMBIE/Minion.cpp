#include "Minion.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Minion::Minion(sf::Vector2f spawnPos)
{
    if (!m_moveTex.loadFromFile("ASSETS/IMAGES/Minionmove.png"))
        std::cout << "Failed to load minion move texture\n";
    else
        std::cout << "Successfully loaded minion move texture\n";
    m_moveTex.setSmooth(false);

    if (!m_attackTex.loadFromFile("ASSETS/IMAGES/Minionattcak.png"))
        std::cout << "Failed to load minion attack texture\n";
    else
        std::cout << "Successfully loaded minion attack texture\n";
    m_attackTex.setSmooth(false);

    if (!m_deathTex.loadFromFile("ASSETS/IMAGES/Miniondeath.png"))
        std::cout << "Failed to load minion death texture\n";
    else
        std::cout << "Successfully loaded minion death texture\n";
    m_deathTex.setSmooth(false);

    m_sprite.setTexture(m_moveTex);
    m_sprite.setTextureRect({ 0, 0, 32, 32 });
    m_sprite.setOrigin(16.f, 16.f);
    m_sprite.setScale(m_baseScale, m_baseScale);
    m_sprite.setPosition(spawnPos);

    // Setup health bars
    m_healthBarBack.setFillColor(sf::Color(60, 0, 0));
    m_healthBarBack.setOutlineThickness(1.f);
    m_healthBarBack.setOutlineColor(sf::Color::Black);
    m_healthBarFront.setFillColor(sf::Color(150, 0, 0));

    m_health = m_maxHealth;
}

void Minion::setPosition(float x, float y)
{
    m_sprite.setPosition(x, y);
}

sf::Vector2f Minion::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect Minion::getBounds() const
{
    return m_sprite.getGlobalBounds();
}

sf::FloatRect Minion::getHitbox() const
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

float Minion::distanceTo(sf::Vector2f p) const
{
    sf::Vector2f d = p - getPosition();
    return std::sqrt(d.x * d.x + d.y * d.y);
}

void Minion::setState(State newState)
{
    if (newState == m_state) return;

    m_prevState = m_state;
    m_state = newState;

    m_frameIndex = 0;
    m_frameTimer = 0.f;

    if (m_state == State::Attack)
    {
        m_attackTimer = 0.f;
        m_hasDealtDamageThisAttack = false;
    }
    else if (m_state == State::Dying)
    {
        m_velocity = { 0.f, 0.f };
        m_deathAnimComplete = false;
        std::cout << "[Minion] Starting death animation\n";
    }
}

void Minion::takeDamage(int dmg)
{
    m_health -= dmg;
    if (m_health < 0) m_health = 0;

    // Trigger hit flash
    m_hitFlashTimer = m_hitFlashDuration;

    if (m_health <= 0 && m_state != State::Dying)
    {
        setState(State::Dying);
    }
}

void Minion::update(sf::Time dt, sf::Vector2f playerPos, 
    const std::vector<std::vector<int>>& tiles, float tileW, float tileH)
{
    float ds = dt.asSeconds();

    if (m_state == State::Dying)
    {
        animate(dt);
        return;
    }

    // Update hit flash timer
    if (m_hitFlashTimer > 0.f)
    {
        m_hitFlashTimer -= ds;
        if (m_hitFlashTimer < 0.f)
            m_hitFlashTimer = 0.f;
    }

    // Update attack cooldown
    if (m_attackCooldown > 0.f)
        m_attackCooldown -= ds;

    float dist = distanceTo(playerPos);

    // State machine logic
    switch (m_state)
    {
    case State::Idle:
        if (dist <= m_detectionRange)
        {
            setState(State::Chase);
        }
        break;

    case State::Chase:
    {
        // Check if in attack range
        if (dist <= m_attackRange && m_attackCooldown <= 0.f)
        {
            setState(State::Attack);
            break;
        }

        // Move toward player
        sf::Vector2f dir = playerPos - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir /= len;
            m_velocity = dir * m_speed;
        }
        break;
    }

    case State::Attack:
    {
        m_attackTimer += ds;

        sf::Vector2f dir = playerPos - getPosition();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir /= len;
            m_velocity = dir * (m_speed * 0.5f);
        }
        else
        {
            m_velocity = { 0.f, 0.f };
        }

        if (m_attackTimer >= m_attackDuration)
        {
            m_attackCooldown = m_attackCooldownDuration;
            setState(State::Chase);
        }
        break;
    }

    default:
        break;
    }

    // Update facing direction
    if (m_velocity != sf::Vector2f{ 0.f, 0.f })
    {
        updateFacing(m_velocity);
    }

    m_sprite.move(m_velocity * ds);
    animate(dt);

   
    if (m_state == State::Chase || m_state == State::Idle)
    {
        m_velocity = { 0.f, 0.f };
    }
}

void Minion::render(sf::RenderWindow& window)
{
    if (m_hitFlashTimer > 0.f)
    {
        float intensity = m_hitFlashTimer / m_hitFlashDuration;
       
        sf::Color flashColor(
            255,
            static_cast<sf::Uint8>(100 + (155 * (1.f - intensity))),
            static_cast<sf::Uint8>(100 + (155 * (1.f - intensity)))
        );
        
        m_sprite.setColor(flashColor);
    }
    else
    {
        m_sprite.setColor(sf::Color::White);
    }

    window.draw(m_sprite);

    // Don't draw health bar if dead
    if (m_state == State::Dying || m_deathAnimComplete)
        return;

    // Draw health bar above minion
    sf::FloatRect b = m_sprite.getGlobalBounds();
    
    float barW = b.width * 0.7f;
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

int Minion::tryDealDamage(const sf::FloatRect& playerHitbox)
{
    if (m_state == State::Dying)
        return 0;

    if (m_state == State::Attack && !m_hasDealtDamageThisAttack)
    {
        if (getHitbox().intersects(playerHitbox))
        {
            m_hasDealtDamageThisAttack = true;
            std::cout << "[Minion] Hit player!\n";
            return 15;  // Minion damage
        }
    }
    
    return 0;
}

void Minion::updateFacing(sf::Vector2f dir)
{
    // Update facing based on horizontal movement
    if (std::abs(dir.x) > 0.01f)
    {
        m_facingLeft = (dir.x < 0.f);
    }
}

void Minion::animate(sf::Time dt)
{
    m_frameTimer += dt.asSeconds();
    if (m_frameTimer < m_frameTime)
        return;

    m_frameTimer = 0.f;
    m_frameIndex++;

    // Death animation
    if (m_state == State::Dying)
    {
        if (m_frameIndex >= m_deathFrameCount)
        {
            m_frameIndex = m_deathFrameCount - 1;
            m_deathAnimComplete = true;
            std::cout << "[Minion] Death animation complete\n";
            return;
        }

        m_sprite.setTexture(m_deathTex);
        m_sprite.setTextureRect({ m_frameIndex * 32, 0, 32, 32 });
        m_sprite.setScale(m_baseScale, m_baseScale);
        return;
    }

    // Attack animation 
    if (m_state == State::Attack)
    {
        m_frameIndex = m_frameIndex % m_attackFrameCount;

        m_sprite.setTexture(m_attackTex);
        m_sprite.setTextureRect({ m_frameIndex * 32, 0, 32, 32 });
        
        // Flip sprite based on facing direction
        if (m_facingLeft)
        {
            m_sprite.setScale(m_baseScale, m_baseScale);
        }
        else
        {
            m_sprite.setScale(-m_baseScale, m_baseScale);
        }
        
        std::cout << "[Minion] Attack frame: " << m_frameIndex << "\n";
        return;
    }

    m_frameIndex = m_frameIndex % m_moveFrameCount;

    m_sprite.setTexture(m_moveTex);
    m_sprite.setTextureRect({ m_frameIndex * 32, 0, 32, 32 });
    
    // Flip sprite based on facing direction
    if (m_facingLeft)
    {
        m_sprite.setScale(m_baseScale, m_baseScale);
    }
    else
    {
        m_sprite.setScale(-m_baseScale, m_baseScale);
    }
}
