#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Minion
{
public:
    enum class State
    {
        Idle,
        Chase,
        Attack,
        Dying
    };

    explicit Minion(sf::Vector2f spawnPos);

    void setPosition(float x, float y);
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
    sf::FloatRect getHitbox() const;

    State getState() const { return m_state; }

    void update(
        sf::Time dt,
        sf::Vector2f playerPos,
        const std::vector<std::vector<int>>& tiles,
        float tileW,
        float tileH
    );
    void render(sf::RenderWindow& window);

    void takeDamage(int dmg);
    bool isDead() const { return m_health <= 0 && m_state != State::Dying; }
    bool isDeathAnimationComplete() const { return m_deathAnimComplete; }

    int tryDealDamage(const sf::FloatRect& playerHitbox);

    void applyDifficulty(int health, int damage);

private:
    void setState(State newState);
    float distanceTo(sf::Vector2f p) const;
    void updateFacing(sf::Vector2f dir);
    void animate(sf::Time dt);

    sf::Sprite m_sprite;

    State m_state{ State::Idle };
    State m_prevState{ State::Idle };

    sf::Vector2f m_velocity{ 0.f, 0.f };
    float m_speed{ 150.f };

    float m_detectionRange{ 700.f };
    float m_attackRange{ 50.f };

    int m_maxHealth{ 50 };
    int m_health{ 50 };
    int m_damage{ 15 };

    float m_attackTimer{ 0.f };
    float m_attackDuration{ 0.4f };
    float m_attackCooldown{ 0.f };
    float m_attackCooldownDuration{ 1.f };

    bool m_hasDealtDamageThisAttack{ false };

    bool m_facingLeft{ true };
    int m_frameIndex{ 0 };
    float m_frameTimer{ 0.f };
    float m_frameTime{ 0.15f };
    float m_baseScale{ 1.8f };

    sf::Texture m_moveTex;
    sf::Texture m_attackTex;
    sf::Texture m_deathTex;

    static const int m_moveFrameCount = 4;
    static const int m_attackFrameCount = 4;
    static const int m_deathFrameCount = 6;
    bool m_deathAnimComplete{ false };

    sf::RectangleShape m_healthBarBack;
    sf::RectangleShape m_healthBarFront;
    float m_healthBarHeight{ 4.f };
    float m_healthBarOffset{ 5.f };

    // Hit flash effect
    float m_hitFlashTimer{ 0.f };
    float m_hitFlashDuration{ 0.15f };
};

