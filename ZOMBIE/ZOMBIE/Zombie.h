#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Zombie
{
public:
    enum class State
    {
        Idle,
        Patrol,
        Chase,
        Attack,
        Cooldown
    };

   // Zombie();
    explicit Zombie(const sf::Texture& texture);

    void setPosition(float x, float y);
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;

    Zombie::State getState() const { return m_state; }


    void update(
        sf::Time dt,
        sf::Vector2f playerPos,
        const std::vector<std::vector<int>>& tiles,
        float tileW,
        float tileH
    );
    void render(sf::RenderWindow& window);

    sf::FloatRect getHitbox() const;

    void reset(sf::Vector2f spawnPos);

    void setFrozen(bool frozen);

    void takeDamage(int dmg); 
    bool isDead() const { return m_health <= 0; }

    int tryDealDamage(const sf::FloatRect& playerHitbox);

private:
    void setState(State newState);
    float distanceTo(sf::Vector2f p) const;

private:
    sf::Sprite m_sprite;
    const sf::Texture* m_texture = nullptr;
    //sf::Texture m_texture;

    State m_state{ State::Idle };
    State m_prevState{ State::Idle };


    sf::Vector2f m_velocity{ 0.f, 0.f };
    float m_speedPatrol{ 60.f };
    float m_speedChase{ 120.f };

    float m_chaseRange{ 320.f };
    float m_attackRange{ 55.f };

    float m_patrolChangeTimer{ 0.f };
    float m_patrolChangeInterval{ 1.2f };
    sf::Vector2f m_patrolDir{ 0.f, 0.f };

    float m_attackTimer{ 0.f };
    float m_attackDuration{ 0.25f };

    float m_cooldownTimer{ 0.f };
    float m_cooldownDuration{ 0.5f };

    bool m_frozen = false;

    int m_health = 100;

    sf::Vector2f m_moveTarget;

    bool m_hasDealtDamageThisAttack{ false };

};

