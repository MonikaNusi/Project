#include "SpikeTrap.h"

SpikeTrap::SpikeTrap(sf::Vector2f position, const sf::Texture& texture)
    : m_texture(&texture)
{
    m_sprite.setTexture(*m_texture);
    m_sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(m_frameWidth), static_cast<int>(m_frameHeight)));
    m_sprite.setPosition(position);
    m_sprite.setOrigin(m_frameWidth / 2.f, m_frameHeight / 2.f);
    m_sprite.setScale(1.5f, 1.5f);  // Scale up for visibility
}

// Copy constructor
SpikeTrap::SpikeTrap(const SpikeTrap& other)
    : m_texture(other.m_texture),
      m_active(other.m_active),
      m_isAnimating(other.m_isAnimating),
      m_currentFrame(other.m_currentFrame),
      m_frameTimer(other.m_frameTimer),
      m_cooldownTimer(other.m_cooldownTimer)
{
    m_sprite.setTexture(*m_texture);
    m_sprite.setTextureRect(other.m_sprite.getTextureRect());
    m_sprite.setPosition(other.m_sprite.getPosition());
    m_sprite.setOrigin(other.m_sprite.getOrigin());
    m_sprite.setScale(other.m_sprite.getScale());
}

// Copy assignment operator
SpikeTrap& SpikeTrap::operator=(const SpikeTrap& other)
{
    if (this != &other)
    {
        m_texture = other.m_texture;
        m_active = other.m_active;
        m_isAnimating = other.m_isAnimating;
        m_currentFrame = other.m_currentFrame;
        m_frameTimer = other.m_frameTimer;
        m_cooldownTimer = other.m_cooldownTimer;
        
        m_sprite.setTexture(*m_texture);
        m_sprite.setTextureRect(other.m_sprite.getTextureRect());
        m_sprite.setPosition(other.m_sprite.getPosition());
        m_sprite.setOrigin(other.m_sprite.getOrigin());
        m_sprite.setScale(other.m_sprite.getScale());
    }
    return *this;
}

void SpikeTrap::update(sf::Time dt)
{
    if (m_cooldownTimer > 0.f)
    {
        m_cooldownTimer -= dt.asSeconds();
        if (m_cooldownTimer <= 0.f)
        {
            m_active = false;
            m_isAnimating = false;
            m_currentFrame = 0;
            m_sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(m_frameWidth), static_cast<int>(m_frameHeight)));
        }
    }

    if (m_isAnimating)
    {
        animate(dt);
    }
}

void SpikeTrap::render(sf::RenderWindow& window)
{
    window.draw(m_sprite);
}

sf::FloatRect SpikeTrap::getBounds() const
{
    sf::FloatRect bounds = m_sprite.getGlobalBounds();
    // Make hitbox slightly smaller for better feel
    float shrink = 8.f;
    return sf::FloatRect(
        bounds.left + shrink,
        bounds.top + shrink,
        bounds.width - shrink * 2.f,
        bounds.height - shrink * 2.f
    );
}

void SpikeTrap::trigger()
{
    if (m_cooldownTimer <= 0.f)
    {
        m_active = true;
        m_isAnimating = true;
        m_currentFrame = 0;
        m_frameTimer = 0.f;
        m_cooldownTimer = m_cooldownDuration;
    }
}

void SpikeTrap::animate(sf::Time dt)
{
    m_frameTimer += dt.asSeconds();

    if (m_frameTimer >= m_frameTime)
    {
        m_frameTimer = 0.f;
        m_currentFrame++;

        if (m_currentFrame >= m_frameCount)
        {
            m_currentFrame = m_frameCount - 1;  // Hold on last frame
        }

        m_sprite.setTextureRect(sf::IntRect(
            m_currentFrame * static_cast<int>(m_frameWidth),
            0,
            static_cast<int>(m_frameWidth),
            static_cast<int>(m_frameHeight)
        ));
    }
}
