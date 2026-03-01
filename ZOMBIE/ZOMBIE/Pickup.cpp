#include "Pickup.h"
#include <cmath>
#include <iostream>

Pickup::Pickup(sf::Vector2f position, Type type, int value, 
               const sf::Texture& closedTexture, const sf::Texture& openTexture)
    : m_closedTexture(&closedTexture),
      m_openTexture(&openTexture),
      m_type(type),
      m_value(value),
      m_baseY(position.y)
{
    // Start with closed texture
    m_sprite.setTexture(*m_closedTexture);
    
    m_sprite.setOrigin(
        closedTexture.getSize().x / 2.f,
        closedTexture.getSize().y / 2.f
    );
    
    // Position the sprite
    m_sprite.setPosition(position);
    m_sprite.setScale(1.5f, 1.5f);
}

// Copy constructor
Pickup::Pickup(const Pickup& other)
    : m_closedTexture(other.m_closedTexture),
      m_openTexture(other.m_openTexture),
      m_type(other.m_type),
      m_value(other.m_value),
      m_picked(other.m_picked),
      m_opened(other.m_opened),
      m_bobTimer(other.m_bobTimer),
      m_baseY(other.m_baseY)
{
    if (m_opened)
    {
        m_sprite.setTexture(*m_openTexture);
        m_sprite.setOrigin(
            m_openTexture->getSize().x / 2.f,
            m_openTexture->getSize().y / 2.f
        );
    }
    else
    {
        m_sprite.setTexture(*m_closedTexture);
        m_sprite.setOrigin(
            m_closedTexture->getSize().x / 2.f,
            m_closedTexture->getSize().y / 2.f
        );
    }
    
    m_sprite.setPosition(other.m_sprite.getPosition());
    m_sprite.setScale(other.m_sprite.getScale());
}

// Copy assignment operator
Pickup& Pickup::operator=(const Pickup& other)
{
    if (this != &other)
    {
        m_closedTexture = other.m_closedTexture;
        m_openTexture = other.m_openTexture;
        m_type = other.m_type;
        m_value = other.m_value;
        m_picked = other.m_picked;
        m_opened = other.m_opened;
        m_bobTimer = other.m_bobTimer;
        m_baseY = other.m_baseY;
        
        // Set the appropriate texture based on opened state
        if (m_opened)
        {
            m_sprite.setTexture(*m_openTexture);
            m_sprite.setOrigin(
                m_openTexture->getSize().x / 2.f,
                m_openTexture->getSize().y / 2.f
            );
        }
        else
        {
            m_sprite.setTexture(*m_closedTexture);
            m_sprite.setOrigin(
                m_closedTexture->getSize().x / 2.f,
                m_closedTexture->getSize().y / 2.f
            );
        }
        
        m_sprite.setPosition(other.m_sprite.getPosition());
        m_sprite.setScale(other.m_sprite.getScale());
    }
    return *this;
}

void Pickup::update(sf::Time dt)
{
    // Chests don't animate - they stay static
    // This function is called from Game::update() but does nothing for now
}

void Pickup::render(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}

sf::FloatRect Pickup::getBounds() const
{
    return m_sprite.getGlobalBounds();
}

void Pickup::open()
{
    if (!m_opened)
    {
        m_opened = true;
        
        // Switch to open texture
        m_sprite.setTexture(*m_openTexture);
        
        // Update origin for new texture
        m_sprite.setOrigin(
            m_openTexture->getSize().x / 2.f,
            m_openTexture->getSize().y / 2.f
        );
    }
}

bool Pickup::isNearby(const sf::FloatRect& playerBox, float radius) const
{
    // Get chest center
    sf::FloatRect bounds = m_sprite.getGlobalBounds();
    sf::Vector2f chestCenter(
        bounds.left + bounds.width / 2.f,
        bounds.top + bounds.height / 2.f
    );
    
    sf::Vector2f playerCenter(
        playerBox.left + playerBox.width / 2.f,
        playerBox.top + playerBox.height / 2.f
    );
    
    float dx = chestCenter.x - playerCenter.x;
    float dy = chestCenter.y - playerCenter.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance < radius;
}

