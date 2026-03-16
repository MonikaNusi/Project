#include "Gun.h"
#include <iostream>

Gun::Gun()
{
    loadTextures();
}

void Gun::loadTextures()
{
    if (!m_gunDownTex.loadFromFile("ASSETS/IMAGES/Gundown.png"))
    {
        std::cout << "Failed to load gun down texture\n";
    }
    m_gunDownTex.setSmooth(false);

    if (!m_gunUpTex.loadFromFile("ASSETS/IMAGES/Gunup.png"))
    {
        std::cout << "Failed to load gun up texture\n";
    }
    m_gunUpTex.setSmooth(false);

    if (!m_gunLeftTex.loadFromFile("ASSETS/IMAGES/Gunleft.png"))
    {
        std::cout << "Failed to load gun left texture\n";
    }
    m_gunLeftTex.setSmooth(false);

    if (!m_gunRightTex.loadFromFile("ASSETS/IMAGES/Gunright.png"))
    {
        std::cout << "Failed to load gun right texture\n";
    }
    m_gunRightTex.setSmooth(false);

    if (!m_flashDownTex.loadFromFile("ASSETS/IMAGES/FireDown.png"))
    {
        std::cout << "Failed to load muzzle flash down texture\n";
    }
    m_flashDownTex.setSmooth(false);

    if (!m_flashUpTex.loadFromFile("ASSETS/IMAGES/FireUp.png"))
    {
        std::cout << "Failed to load muzzle flash up texture\n";
    }
    m_flashUpTex.setSmooth(false);

    if (!m_flashLeftTex.loadFromFile("ASSETS/IMAGES/FireLeft.png"))
    {
        std::cout << "Failed to load muzzle flash left texture\n";
    }
    m_flashLeftTex.setSmooth(false);

    if (!m_flashRightTex.loadFromFile("ASSETS/IMAGES/FireRight.png"))
    {
        std::cout << "Failed to load muzzle flash right texture\n";
    }
    m_flashRightTex.setSmooth(false);

    // Set initial gun texture and frame
    m_sprite.setTexture(m_gunDownTex);
    m_sprite.setTextureRect(sf::IntRect(0, 0, 5, 17));
    m_sprite.setScale(m_baseScale, m_baseScale);
    m_sprite.setOrigin(2.5f, 8.5f);
}

void Gun::setDirection(Direction dir)
{
    if (m_currentDirection == dir) return;

    m_currentDirection = dir;
    
    if (!m_isAnimating)
    {
        m_frameIndex = 0;
    }

    switch (m_currentDirection)
    {
    case Direction::Down:
        m_sprite.setTexture(m_gunDownTex);
        m_sprite.setOrigin(2.5f, 8.5f);
        if (!m_isAnimating)
        {
            m_sprite.setTextureRect(sf::IntRect(0, 0, 5, 17));
        }
        break;
    case Direction::Up:
        m_sprite.setTexture(m_gunUpTex);
        m_sprite.setOrigin(2.5f, 8.5f);
        if (!m_isAnimating)
        {
            m_sprite.setTextureRect(sf::IntRect(0, 0, 5, 17));
        }
        break;
    case Direction::Left:
        m_sprite.setTexture(m_gunLeftTex);
        m_sprite.setOrigin(9.f, 5.f);
        if (!m_isAnimating)
        {
            m_sprite.setTextureRect(sf::IntRect(0, 0, 18, 10));
        }
        break;
    case Direction::Right:
        m_sprite.setTexture(m_gunRightTex);
        m_sprite.setOrigin(9.f, 5.f);
        if (!m_isAnimating)
        {
            m_sprite.setTextureRect(sf::IntRect(0, 0, 18, 10));
        }
        break;
    }
}

void Gun::setPosition(sf::Vector2f playerPos, sf::Vector2f playerSize)
{
    sf::Vector2f gunPos = playerPos;

    switch (m_currentDirection)
    {
    case Direction::Down:
        gunPos.x += playerSize.x * 0.5f;
        gunPos.y += playerSize.y * 0.7f;
        break;
    case Direction::Up:
        gunPos.x += playerSize.x * 0.5f;
        gunPos.y += playerSize.y * 0.3f;
        break;
    case Direction::Left:
        gunPos.x += playerSize.x * 0.3f;
        gunPos.y += playerSize.y * 0.5f;
        break;
    case Direction::Right:
        gunPos.x += playerSize.x * 0.7f;
        gunPos.y += playerSize.y * 0.5f;
        break;
    }

    m_sprite.setPosition(gunPos);
    
    if (m_flashPlaying)
    {
        m_flashSprite.setPosition(getBarrelPosition());
    }
}

void Gun::update(sf::Time dt)
{
    if (!m_isAnimating)
    {
        switch (m_currentDirection)
        {
        case Direction::Down:
        case Direction::Up:
            m_sprite.setTextureRect(sf::IntRect(0, 0, 5, 17));
            break;
        case Direction::Left:
        case Direction::Right:
            m_sprite.setTextureRect(sf::IntRect(0, 0, 18, 10));
            break;
        }
    }
    else
    {
        m_frameTimer += dt.asSeconds();

        if (m_frameTimer >= m_frameTime)
        {
            m_frameTimer = 0.f;
            m_frameIndex++;

            if (m_frameIndex >= m_frameCount)
            {
                m_frameIndex = 0;
                m_isAnimating = false;
              
                switch (m_currentDirection)
                {
                case Direction::Down:
                case Direction::Up:
                    m_sprite.setTextureRect(sf::IntRect(0, 0, 5, 17));
                    break;
                case Direction::Left:
                case Direction::Right:
                    m_sprite.setTextureRect(sf::IntRect(0, 0, 18, 10));
                    break;
                }
            }
            else
            {
                switch (m_currentDirection)
                {
                case Direction::Down:
                case Direction::Up:
                    m_sprite.setTextureRect(sf::IntRect(m_frameIndex * 5, 0, 5, 17));
                    break;
                case Direction::Left:
                case Direction::Right:
                    m_sprite.setTextureRect(sf::IntRect(m_frameIndex * 18, 0, 18, 10));
                    break;
                }
            }
        }
    }
   
    if (m_flashPlaying)
    {
        m_flashFrameTimer += dt.asSeconds();
        
        if (m_flashFrameTimer >= m_flashFrameTime)
        {
            m_flashFrameTimer = 0.f;
            m_flashFrameIndex++;
            
            if (m_flashFrameIndex >= m_flashFrameCount)
            {
                m_flashFrameIndex = 0;
                m_flashPlaying = false;
            }
            else
            {
                // Update flash texture rect based on frame and direction
                switch (m_currentDirection)
                {
                case Direction::Down:
                case Direction::Up:
                    m_flashSprite.setTextureRect(sf::IntRect(m_flashFrameIndex * 7, 0, 7, 10));
                    break;
                case Direction::Left:
                case Direction::Right:
                    m_flashSprite.setTextureRect(sf::IntRect(m_flashFrameIndex * 10, 0, 10, 7));
                    break;
                }
            }
        }
    }
}

void Gun::render(sf::RenderWindow& window)
{
    window.draw(m_sprite);
    
    if (m_flashPlaying)
    {
        window.draw(m_flashSprite);
    }
}

void Gun::playShootAnimation()
{
    // Start gun shoot animation
    m_isAnimating = true;
    m_frameIndex = 0;
    m_frameTimer = 0.f;
    
    // Start muzzle flash animation
    m_flashPlaying = true;
    m_flashFrameIndex = 0;
    m_flashFrameTimer = 0.f;

    sf::Vector2f barrelPos = getBarrelPosition();
    
    switch (m_currentDirection)
    {
    case Direction::Down:
        m_flashSprite.setTexture(m_flashDownTex);
        m_flashSprite.setTextureRect(sf::IntRect(0, 0, 7, 10));
        m_flashSprite.setOrigin(3.5f, 5.f);
        m_flashSprite.setScale(3.f, 3.f);
        break;
    case Direction::Up:
        m_flashSprite.setTexture(m_flashUpTex);
        m_flashSprite.setTextureRect(sf::IntRect(0, 0, 7, 10));
        m_flashSprite.setOrigin(3.5f, 5.f);
        m_flashSprite.setScale(3.f, 3.f);
        break;
    case Direction::Left:
        m_flashSprite.setTexture(m_flashLeftTex);
        m_flashSprite.setTextureRect(sf::IntRect(0, 0, 10, 7));
        m_flashSprite.setOrigin(5.f, 3.5f);
        m_flashSprite.setScale(3.f, 3.f);
        break;
    case Direction::Right:
        m_flashSprite.setTexture(m_flashRightTex);
        m_flashSprite.setTextureRect(sf::IntRect(0, 0, 10, 7));
        m_flashSprite.setOrigin(5.f, 3.5f);
        m_flashSprite.setScale(3.f, 3.f);
        break;
    }
    
    m_flashSprite.setPosition(barrelPos);
}

sf::Vector2f Gun::getBarrelPosition() const
{
    // Use the sprite's current texture rect and transform to compute an accurate barrel/world position.
    // This accounts for the sprite's origin, scale and any per-frame textureRect changes.
    sf::IntRect texRect = m_sprite.getTextureRect();

    // Local coordinates (relative to the sprite's local space - top-left of the textureRect).
    // Add a small padding (4 px) so the barrel is slightly outside the gun sprite.
    const float padding = 4.f;

    sf::Vector2f localPoint;

    if (texRect.width == 0 || texRect.height == 0)
    {
        // fallback - should not happen but safe guard
        return m_sprite.getPosition();
    }

    switch (m_currentDirection)
    {
    case Direction::Down:
        // middle bottom of the texture rect, just below
        localPoint = sf::Vector2f(static_cast<float>(texRect.left) + texRect.width * 0.5f,
                                  static_cast<float>(texRect.top + texRect.height) + padding);
        break;
    case Direction::Up:
        // middle top of the texture rect, just above
        localPoint = sf::Vector2f(static_cast<float>(texRect.left) + texRect.width * 0.5f,
                                  static_cast<float>(texRect.top) - padding);
        break;
    case Direction::Left:
        // left middle, just left of the texture rect
        localPoint = sf::Vector2f(static_cast<float>(texRect.left) - padding,
                                  static_cast<float>(texRect.top) + texRect.height * 0.5f);
        break;
    case Direction::Right:
        // right middle, just right of the texture rect
        localPoint = sf::Vector2f(static_cast<float>(texRect.left + texRect.width) + padding,
                                  static_cast<float>(texRect.top) + texRect.height * 0.5f);
        break;
    }

    // Transform the local point into world coordinates using the sprite transform
    return m_sprite.getTransform().transformPoint(localPoint);
}