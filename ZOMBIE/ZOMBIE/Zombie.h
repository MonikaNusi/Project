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


    enum class Anim
    {
        Walk,
        Attack
    };

    enum class Facing
    {
        Down,
        Side,
        Up
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

    void updateFacing(sf::Vector2f dir);
    void Zombie::animate(sf::Time dt);

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

    int m_maxHealth{ 100 };
    int m_health{ 100 };

    sf::Vector2f m_moveTarget;

    bool m_hasDealtDamageThisAttack{ false };

    // Animation state
    Anim m_anim = Anim::Walk;
    Facing m_facing = Facing::Down;

    // Frame timing
    int m_frameIndex = 0;
    float m_frameTimer = 0.f;
    float m_frameTime = 0.14f;

    float m_baseScale = 2.f;

    // Textures
    sf::Texture m_walkDownTex;
    sf::Texture m_walkUpTex;
    sf::Texture m_walkSideTex;

    sf::Texture m_attackDownTex;
    sf::Texture m_attackUpTex;
    sf::Texture m_attackSideTex;

    sf::RectangleShape m_healthBarBack;
    sf::RectangleShape m_healthBarFront;
    float m_healthBarHeight{ 6.f };
    float m_healthBarOffset{ 6.f };

};

