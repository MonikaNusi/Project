#include "BossZombie.h"
#include <cmath>
#include <iostream>
#include <algorithm>

sf::SoundBuffer BossZombie::m_roarSoundBuffer;
sf::SoundBuffer BossZombie::m_deathSoundBuffer;
sf::SoundBuffer BossZombie::m_chargeSoundBuffer;

BossZombie::BossZombie(sf::Vector2f spawnPos)
{
	m_walkDownTex.loadFromFile("ASSETS/IMAGES/ZombieDownWalk1.png");
	m_walkDownTex.setSmooth(false);
	m_walkUpTex.loadFromFile("ASSETS/IMAGES/ZombieUpWalk1.png");
	m_walkUpTex.setSmooth(false);
	m_walkSideTex.loadFromFile("ASSETS/IMAGES/ZombieSideWalk1.png");
	m_walkSideTex.setSmooth(false);

	m_attackDownTex.loadFromFile("ASSETS/IMAGES/ZombieDownAttack1.png");
	m_attackDownTex.setSmooth(false);
	m_attackUpTex.loadFromFile("ASSETS/IMAGES/ZombieUpAttack1.png");
	m_attackUpTex.setSmooth(false);
	m_attackSideTex.loadFromFile("ASSETS/IMAGES/ZombieSideAttack1.png");
	m_attackSideTex.setSmooth(false);

	if (!m_deathTex.loadFromFile("ASSETS/IMAGES/ZombieDeath1.png"))
	{
		std::cout << "Failed to load boss zombie death animation\n";
	}
	m_deathTex.setSmooth(false);

	m_sprite.setTexture(m_walkDownTex);
	m_sprite.setTextureRect({ 0, 0, 16, 28 });
	m_sprite.setOrigin(8.f, 14.f);
	m_sprite.setScale(m_baseScale, m_baseScale);
	m_sprite.setPosition(spawnPos);

	static bool soundsLoaded = false;
	if (!soundsLoaded)
	{
		if (!m_roarSoundBuffer.loadFromFile("ASSETS/SOUNDS/BossRoar.wav"))
		{
			std::cout << "Failed to load boss roar sound\n";
		}

		if (!m_deathSoundBuffer.loadFromFile("ASSETS/SOUNDS/BossDeath.wav"))
		{
			std::cout << "Failed to load boss death sound\n";
		}

		if (!m_chargeSoundBuffer.loadFromFile("ASSETS/SOUNDS/BossCharge.wav"))
		{
			std::cout << "Failed to load boss charge sound\n";
		}

		soundsLoaded = true;
	}

	m_roarSound.setBuffer(m_roarSoundBuffer);
	m_roarSound.setVolume(60.f);

	m_deathSound.setBuffer(m_deathSoundBuffer);
	m_deathSound.setVolume(70.f);

	m_chargeSound.setBuffer(m_chargeSoundBuffer);
	m_chargeSound.setVolume(50.f);

	// Setup health bars
	m_healthBarBack.setFillColor(sf::Color(80, 0, 0));
	m_healthBarBack.setOutlineThickness(2.f);
	m_healthBarBack.setOutlineColor(sf::Color::Black);

	m_healthBarFront.setFillColor(sf::Color(200, 0, 0));

	m_health = m_maxHealth;
}

void BossZombie::setPosition(float x, float y)
{
	m_sprite.setPosition(x, y);
}

sf::Vector2f BossZombie::getPosition() const
{
	return m_sprite.getPosition();
}

sf::FloatRect BossZombie::getBounds() const
{
	return m_sprite.getGlobalBounds();
}

sf::FloatRect BossZombie::getHitbox() const
{
	sf::FloatRect b = m_sprite.getGlobalBounds();
	
	float w = b.width * 0.35f;
	float h = b.height * 0.25f;

	return sf::FloatRect(
		b.left + (b.width - w) * 0.5f,
		b.top + b.height - h,
		w,
		h
	);
}

float BossZombie::distanceTo(sf::Vector2f p) const
{
	sf::Vector2f d = p - getPosition();
	return std::sqrt(d.x * d.x + d.y * d.y);
}

void BossZombie::setState(State newState)
{
	if (newState == m_state) return;

	m_prevState = m_state;
	m_state = newState;

	std::cout << "[BossZombie] State change to " << static_cast<int>(m_state) << "\n";

	m_frameIndex = 0;
	m_frameTimer = 0.f;

	if (m_state == State::BigAttack)
	{
		m_attackTimer = 0.f;
		m_hasDealtDamageThisAttack = false;
		if (m_roarSoundBuffer.getDuration().asSeconds() > 0.f)
			m_roarSound.play();
	}
	else if (m_state == State::Charging)
	{
		m_chargeTimer = 0.f;
		if (m_chargeSoundBuffer.getDuration().asSeconds() > 0.f)
			m_chargeSound.play();
    }
	else if (m_state == State::SummonMinions)
	{
		m_summonTimer = 0.f;
		if (m_roarSoundBuffer.getDuration().asSeconds() > 0.f)
			m_roarSound.play();
		std::cout << "[BossZombie] Starting minion summon!\n";
	}
	else if (m_state == State::Dying)
	{
		m_velocity = { 0.f, 0.f };
		m_deathAnimComplete = false;
		if (m_deathSoundBuffer.getDuration().asSeconds() > 0.f)
			m_deathSound.play();
		std::cout << "[BossZombie] Starting death animation\n";
	}
}

void BossZombie::takeDamage(int dmg)
{
	m_health -= dmg;
	if (m_health < 0) m_health = 0;

	// Trigger hit flash
	m_hitFlashTimer = m_hitFlashDuration;

	if (m_health <= 0 && m_state != State::Dying)
	{
		setState(State::Dying);
	}
}

void BossZombie::update(sf::Time dt, sf::Vector2f playerPos, 
	const std::vector<std::vector<int>>& tiles, float tileW, float tileH)
{
	float ds = dt.asSeconds();

	if (m_state == State::Dying)
	{
		animate(dt);
		return;
	}

	// Update hit flash timer
	if (m_hitFlashTimer > 0.f)
	{
		m_hitFlashTimer -= ds;
		if (m_hitFlashTimer < 0.f)
			m_hitFlashTimer = 0.f;
	}

	// Update cooldowns
	if (m_attackCooldown > 0.f)
		m_attackCooldown -= ds;

	if (m_chargeCooldown > 0.f)
		m_chargeCooldown -= ds;

	if (m_summonCooldown > 0.f)
		m_summonCooldown -= ds;

	// Update roar timer
	m_roarTimer += ds;
	if (m_roarTimer >= m_roarInterval && m_state != State::Idle)
	{
		m_roarTimer = 0.f;
		if (m_roarSoundBuffer.getDuration().asSeconds() > 0.f)
			m_roarSound.play();
	}

	float dist = distanceTo(playerPos);

	// State machine logic
	switch (m_state)
	{
	case State::Idle:
		if (dist <= m_detectionRange)
		{
			setState(State::Walking);
		}
		break;

	case State::Walking:
	{
		// Check if should summon minions
		if (!m_hasSpawnedMinionsAtThreshold && m_health <= m_healthThresholdForSummon)
		{
			setState(State::SummonMinions);
			m_hasSpawnedMinionsAtThreshold = true;
			break;
		}
		else if (m_summonCooldown <= 0.f && (std::rand() % 100) < 5) 
		{
			setState(State::SummonMinions);
			break;
		}

		// Check if should charge
		if (dist <= m_chargeRange && dist > m_attackRange && m_chargeCooldown <= 0.f)
		{
			sf::Vector2f dir = playerPos - getPosition();
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.001f)
			{
				m_chargeDirection = dir / len;
				setState(State::Charging);
				break;
			}
		}

		// Check if in attack range
		if (dist <= m_attackRange && m_attackCooldown <= 0.f)
		{
			setState(State::BigAttack);
			break;
		}

		// Move toward player
		sf::Vector2f dir = playerPos - getPosition();
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.001f)
		{
			dir /= len;
			m_velocity = dir * m_speedWalk;
		}
		break;
	}

	case State::Charging:
	{
		m_chargeTimer += ds;
		m_velocity = m_chargeDirection * m_speedCharge;

		if (m_chargeTimer >= m_chargeDuration)
		{
			m_chargeCooldown = m_chargeCooldownDuration;
			setState(State::Walking);
		}
		break;
	}

	case State::BigAttack:
	{
		m_attackTimer += ds;

		// Lunge toward player during attack
		sf::Vector2f dir = playerPos - getPosition();
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.001f)
		{
			dir /= len;
			m_velocity = dir * (m_speedWalk * 1.5f);
		}
		else
		{
			m_velocity = { 0.f, 0.f };
		}

		if (m_attackTimer >= m_attackDuration)
		{
			m_attackCooldown = m_attackCooldownDuration;
			setState(State::Walking);
		}
		break;
	}

	case State::SummonMinions:
	{
		m_summonTimer += ds;
		m_velocity = { 0.f, 0.f };  // Stand still while summoning

		if (m_summonTimer >= m_summonDuration)
		{
			m_shouldSpawnMinions = true;
			m_summonCooldown = m_summonCooldownDuration;
			setState(State::Walking);
			std::cout << "[BossZombie] Minions ready to spawn!\n";
		}
		break;
	}

	default:
		break;
	}
		
	// Update facing direction
	if (m_velocity != sf::Vector2f{ 0.f, 0.f })
	{
		updateFacing(m_velocity);
	}

	m_sprite.move(m_velocity * ds);

	animate(dt);

	if (m_state != State::Charging && m_state != State::BigAttack)
	{
		m_velocity = { 0.f, 0.f };
	}
}

void BossZombie::render(sf::RenderWindow& window)
{
	if (m_hitFlashTimer > 0.f)
	{
		float intensity = m_hitFlashTimer / m_hitFlashDuration;
		
		sf::Color flashColor(
			255,
			static_cast<sf::Uint8>(100 + (155 * (1.f - intensity))),
			static_cast<sf::Uint8>(100 + (155 * (1.f - intensity)))
		);
		
		m_sprite.setColor(flashColor);
	}
	else
	{
		m_sprite.setColor(sf::Color::White);
	}

	window.draw(m_sprite);

	// Don't draw health bar if dead
	if (m_state == State::Dying || m_deathAnimComplete)
		return;

	// Draw health bar above the boss
	sf::FloatRect b = m_sprite.getGlobalBounds();
	
	float barW = b.width * 0.8f;
	float barH = m_healthBarHeight;
	float x = b.left + (b.width - barW) * 0.5f;
	float y = b.top - barH - m_healthBarOffset;

	float healthPercent = 0.f;
	if (m_maxHealth > 0)
		healthPercent = std::max(0, m_health) / static_cast<float>(m_maxHealth);

	m_healthBarBack.setSize({ barW, barH });
	m_healthBarBack.setPosition(x, y);

	m_healthBarFront.setSize({ barW * healthPercent, barH });
	m_healthBarFront.setPosition(x, y);

	window.draw(m_healthBarBack);
	window.draw(m_healthBarFront);
}

int BossZombie::tryDealDamage(const sf::FloatRect& playerHitbox)
{
	if (m_state == State::Dying)
		return 0;

	// Attack damage
	if (m_state == State::BigAttack && !m_hasDealtDamageThisAttack)
	{
		if (getHitbox().intersects(playerHitbox))
		{
			m_hasDealtDamageThisAttack = true;
			std::cout << "[BossZombie] Hit player with attack!\n";
			return 30;  // Boss deals more damage
		}
	}
	
	// Charge damage
	if (m_state == State::Charging && !m_hasDealtDamageThisAttack)
	{
		if (getHitbox().intersects(playerHitbox))
		{
			m_hasDealtDamageThisAttack = true;
			std::cout << "[BossZombie] Hit player with charge!\n";
			return 20;
		}
	}
	
	return 0;
}

void BossZombie::updateFacing(sf::Vector2f dir)
{
	if (std::abs(dir.x) > std::abs(dir.y))
		m_facing = Facing::Side;
	else if (dir.y < 0)
		m_facing = Facing::Up;
	else
		m_facing = Facing::Down;
}

void BossZombie::animate(sf::Time dt)
{
	m_frameTimer += dt.asSeconds();
	if (m_frameTimer < m_frameTime)
		return;

	m_frameTimer = 0.f;
	m_frameIndex++;


	if (m_state == State::Dying)
	{
		if (m_frameIndex >= m_deathFrameCount)
		{
			m_frameIndex = m_deathFrameCount - 1;
			m_deathAnimComplete = true;
			std::cout << "[BossZombie] Death animation complete\n";
			return;
		}

		m_sprite.setTexture(m_deathTex);
		m_sprite.setTextureRect({ m_frameIndex * 29, 0, 29, 24 });
		m_sprite.setOrigin(14.5f, 12.f);
		m_sprite.setScale(m_baseScale, m_baseScale);
		return;
	}

	// Attack animation
	if (m_state == State::BigAttack || m_state == State::SummonMinions)
	{
		m_frameIndex = std::min(m_frameIndex, 14);

		if (m_facing == Facing::Down)
		{
			m_sprite.setTexture(m_attackDownTex);
			m_sprite.setTextureRect({ m_frameIndex * 22, 0, 22, 37 });
			m_sprite.setOrigin(11.f, 18.5f);
			m_sprite.setScale(m_baseScale, m_baseScale);
		}
		else if (m_facing == Facing::Up)
		{
			m_sprite.setTexture(m_attackUpTex);
			m_sprite.setTextureRect({ m_frameIndex * 27, 0, 27, 27 });
			m_sprite.setOrigin(13.5f, 13.5f);
			m_sprite.setScale(m_baseScale, m_baseScale);
		}
		else
		{
			m_sprite.setTexture(m_attackSideTex);
			m_sprite.setTextureRect({ m_frameIndex * 29, 0, 29, 29 });
			m_sprite.setOrigin(14.5f, 14.5f);
			float sign = (m_velocity.x < 0.f) ? -1.f : 1.f;
			m_sprite.setScale(sign * m_baseScale, m_baseScale);
		}
		return;
	}


	m_frameIndex %= 8;

	if (m_facing == Facing::Down)
	{
		m_sprite.setTexture(m_walkDownTex);
		m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 28 });
		m_sprite.setOrigin(8.f, 14.f);
		m_sprite.setScale(m_baseScale, m_baseScale);
	}
	else if (m_facing == Facing::Up)
	{
		m_sprite.setTexture(m_walkUpTex);
		m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 33 });
		m_sprite.setOrigin(8.f, 16.5f);
		m_sprite.setScale(m_baseScale, m_baseScale);
	}
	else
	{
		m_sprite.setTexture(m_walkSideTex);
		m_sprite.setTextureRect({ m_frameIndex * 16, 0, 16, 33 });
		m_sprite.setOrigin(8.f, 16.5f);
		float sign = (m_velocity.x < 0.f) ? -1.f : 1.f;
		m_sprite.setScale(sign * m_baseScale, m_baseScale);
	}
}


void BossZombie::applyDifficulty(int health)
{
	m_maxHealth = health;
	m_health = health;
	m_healthThresholdForSummon = health / 2;
}