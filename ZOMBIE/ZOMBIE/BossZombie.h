#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class BossZombie
{
public:
	enum class State
	{
		Idle,
		Walking,
		Charging,
		BigAttack,
		Dying
	};

	enum class Facing
	{
		Down,
		Side,
		Up
	};

	explicit BossZombie(sf::Vector2f spawnPos);

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

private:
	void setState(State newState);
	float distanceTo(sf::Vector2f p) const;
	void updateFacing(sf::Vector2f dir);
	void animate(sf::Time dt);

	sf::Sprite m_sprite;

	State m_state{ State::Idle };
	State m_prevState{ State::Idle };

	sf::Vector2f m_velocity{ 0.f, 0.f };
	
	float m_speedWalk{ 80.f };
	float m_speedCharge{ 250.f };

	float m_detectionRange{ 400.f };
	float m_attackRange{ 120.f };
	float m_chargeRange{ 300.f };

	int m_maxHealth{ 500 };
	int m_health{ 500 };

	float m_attackTimer{ 0.f };
	float m_attackDuration{ 0.5f };
	float m_attackCooldown{ 0.f };
	float m_attackCooldownDuration{ 2.f };

	float m_chargeTimer{ 0.f };
	float m_chargeDuration{ 1.5f };
	float m_chargeCooldown{ 0.f };
	float m_chargeCooldownDuration{ 5.f };
	sf::Vector2f m_chargeDirection{ 0.f, 0.f };

	bool m_hasDealtDamageThisAttack{ false };

	Facing m_facing{ Facing::Down };
	int m_frameIndex{ 0 };
	float m_frameTimer{ 0.f };
	float m_frameTime{ 0.14f };
	float m_baseScale{ 5.f };

	sf::Texture m_walkDownTex;
	sf::Texture m_walkUpTex;
	sf::Texture m_walkSideTex;
	sf::Texture m_attackDownTex;
	sf::Texture m_attackUpTex;
	sf::Texture m_attackSideTex;
	sf::Texture m_deathTex;
	
	static const int m_deathFrameCount = 8;
	bool m_deathAnimComplete{ false };

	sf::RectangleShape m_healthBarBack;
	sf::RectangleShape m_healthBarFront;
	float m_healthBarHeight{ 10.f };
	float m_healthBarOffset{ 15.f };

	static sf::SoundBuffer m_roarSoundBuffer;
	static sf::SoundBuffer m_deathSoundBuffer;
	static sf::SoundBuffer m_chargeSoundBuffer;
	sf::Sound m_roarSound;
	sf::Sound m_deathSound;
	sf::Sound m_chargeSound;

	float m_roarTimer{ 0.f };
	float m_roarInterval{ 4.f };	
};