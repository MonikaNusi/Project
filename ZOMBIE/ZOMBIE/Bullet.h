#pragma once
#include <SFML/Graphics.hpp>

class Bullet
{
public:
    Bullet(sf::Vector2f startPos, sf::Vector2f direction);
    
    void update(sf::Time dt);
    void render(sf::RenderWindow& window) const;
    
    bool isAlive() const { return m_alive; }
    void kill() { m_alive = false; }
    
    sf::FloatRect getBounds() const;
    
    static void loadTexture();
    
private:
    sf::Sprite m_sprite;
    static sf::Texture m_texture; 
    static bool m_textureLoaded;
    
    sf::Vector2f m_velocity;
    float m_speed{ 600.f };
    float m_lifetime{ 3.f };
    float m_age{ 0.f };
    bool m_alive{ true };
};
