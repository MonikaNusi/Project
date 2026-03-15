#include "Bullet.h"
#include <cmath>
#include <iostream>

sf::Texture Bullet::m_texture;
bool Bullet::m_textureLoaded = false;

void Bullet::loadTexture()
{
    if (!m_textureLoaded)
    {
        if (!m_texture.loadFromFile("ASSETS/IMAGES/Bullet.png"))
        {
            std::cout << "Failed to load bullet texture\n";
        }
        else
        {
            m_texture.setSmooth(false);
            m_textureLoaded = true;
            std::cout << "Bullet texture loaded successfully\n";
        }
    }
}

Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f direction)
{
    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0.f)
    {
        direction /= length;
    }
    
    m_velocity = direction * m_speed;
    
    // Setup sprite
    if (m_textureLoaded)
    {
        m_sprite.setTexture(m_texture);
        
        // Calculate rotation angle to point in direction of travel
        float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;
        m_sprite.setRotation(angle);
        
        // Center origin
        sf::Vector2u texSize = m_texture.getSize();
        m_sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        
        // Scale bullet
        m_sprite.setScale(5.f, 5.f);
    }
    
    m_sprite.setPosition(startPos);
}

void Bullet::update(sf::Time dt)
{
    if (!m_alive) return;
    
    m_sprite.move(m_velocity * dt.asSeconds());
    
    m_age += dt.asSeconds();
    if (m_age >= m_lifetime)
    {
        m_alive = false;
    }
}

void Bullet::render(sf::RenderWindow& window) const
{
    if (m_alive && m_textureLoaded)
    {
        window.draw(m_sprite);
    }
}

sf::FloatRect Bullet::getBounds() const
{
    return m_sprite.getGlobalBounds();
}
