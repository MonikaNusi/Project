#pragma once

#include <SFML/Graphics.hpp>

class SpikeTrap
{
public:
    SpikeTrap(sf::Vector2f position, const sf::Texture& texture);
    
    // Copy constructor
    SpikeTrap(const SpikeTrap& other);
    
    // Copy assignment operator
    SpikeTrap& operator=(const SpikeTrap& other);


    SpikeTrap(SpikeTrap&& other) noexcept = default;
   
    SpikeTrap& operator=(SpikeTrap&& other) noexcept = default;

    void update(sf::Time dt);
    void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }

    bool isActive() const { return m_active; }
    int getDamage() const { return m_damage; }

    void trigger();  // Activate trap when player steps on it

private:
    sf::Sprite m_sprite;
    const sf::Texture* m_texture;

    bool m_active{ false };
    bool m_isAnimating{ false };

    int m_currentFrame{ 0 };
    const int m_frameCount{ 7 };
    const float m_frameWidth{ 32.f };  // 224 / 7 = 32
    const float m_frameHeight{ 32.f };
    float m_frameTimer{ 0.f };
    const float m_frameTime{ 0.1f };  // Animation speed

    int m_damage{ 5 };
    float m_cooldownTimer{ 0.f };
    const float m_cooldownDuration{ 2.f };  // Time before trap can damage again

    void animate(sf::Time dt);
};

