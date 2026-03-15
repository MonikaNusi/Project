#pragma once

#include <SFML/Graphics.hpp>

class Gun
{
public:
    enum class Direction
    {
        Down,
        Up,
        Left,
        Right
    };

    Gun();

    void loadTextures();
    void setDirection(Direction dir);
    void setPosition(sf::Vector2f playerPos, sf::Vector2f playerSize);
    void update(sf::Time dt);
    void render(sf::RenderWindow& window);
    
    void playShootAnimation();
    bool isAnimating() const { return m_isAnimating; }
    
    sf::Vector2f getBarrelPosition() const;

private:
    // Gun sprites
    sf::Sprite m_sprite;
    sf::Texture m_gunDownTex;
    sf::Texture m_gunUpTex;
    sf::Texture m_gunLeftTex;
    sf::Texture m_gunRightTex;

    Direction m_currentDirection{ Direction::Down };
    
    // Gun animation
    int m_frameIndex{ 0 };
    float m_frameTimer{ 0.f };
    float m_frameTime{ 0.1f };
    int m_frameCount{ 3 };
    bool m_isAnimating{ false };
    
    float m_baseScale{ 2.5f };
    
    // Muzzle flash
    sf::Sprite m_flashSprite;
    sf::Texture m_flashDownTex;
    sf::Texture m_flashUpTex;
    sf::Texture m_flashLeftTex;
    sf::Texture m_flashRightTex;
    
    int m_flashFrameIndex{ 0 };
    float m_flashFrameTimer{ 0.f };
    float m_flashFrameTime{ 0.05f };
    int m_flashFrameCount{ 3 };
    bool m_flashPlaying{ false };
    
    sf::Vector2f m_offset;
};