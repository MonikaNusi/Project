#pragma once
#include <SFML/Graphics.hpp>

class Pickup
{
public:
    enum class Type { Health, Ammo };

    Pickup(sf::Vector2f position, Type type, int value, 
           const sf::Texture& closedTexture, const sf::Texture& openTexture);

    // Copy constructor
    Pickup(const Pickup& other);

    // Copy assignment operator
    Pickup& operator=(const Pickup& other);

    // Move constructor
    Pickup(Pickup&& other) noexcept = default;

    // Move assignment operator
    Pickup& operator=(Pickup&& other) noexcept = default;

    void update(sf::Time dt);
    void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }

    Type getType() const { return m_type; }
    int getValue() const { return m_value; }

    bool isPicked() const { return m_picked; }
    void setPicked(bool picked) { m_picked = picked; }
    
    bool isOpened() const { return m_opened; }
    void open();
    
    bool isNearby(const sf::FloatRect& playerBox, float radius = 50.f) const;

private:
    sf::Sprite m_sprite;
    const sf::Texture* m_closedTexture;
    const sf::Texture* m_openTexture;

    Type m_type;
    int m_value;  // Amount to restore (HP or ammo)
    bool m_picked{ false };
    bool m_opened{ false };

    float m_bobTimer{ 0.f };  
    float m_baseY;
};