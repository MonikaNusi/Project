#include "Player.h"
#include <iostream>

Player::Player()
{
	if (!m_texture.loadFromFile("ASSETS\\IMAGES\\walk.png"))
	{
		std::cout << "Failed to load player texture\n";
	}
	m_sprite.setTexture(m_texture);
	m_sprite.setTextureRect(sf::IntRect(0, 0, m_frameSize.x, m_frameSize.y));
	m_sprite.setPosition(400.f, 400.f);
	m_sprite.setScale(2.7, 2.7);

	// Load footstep sound
	if (!m_footstepBuffer.loadFromFile("ASSETS\\SOUNDS\\Playerwalking.wav"))
	{
		std::cout << "Failed to load footstep sound\n";
	}
	m_footstepSound.setBuffer(m_footstepBuffer);
	m_footstepSound.setVolume(30.f);

	// Load shooting sound
	if (!m_shootBuffer.loadFromFile("ASSETS\\SOUNDS\\Gun.wav"))
	{
		std::cout << "Failed to load shooting sound\n";
	}
	m_shootSound.setBuffer(m_shootBuffer);
	m_shootSound.setVolume(30.f);


	m_gun.loadTextures();
}

void Player::hadnleInput()
{
	 m_velocity = { 0.f, 0.f };

    bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

    // Build velocity vector
    if (up)    m_velocity.y = -m_speed;
    if (down)  m_velocity.y =  m_speed;
    if (left)  m_velocity.x = -m_speed;
    if (right) m_velocity.x =  m_speed;

	if (m_velocity.x != 0 && m_velocity.y != 0)
	{
		m_velocity /= std::sqrt(2.f);
	}
    
	// Track if player is moving
	m_isMoving = (m_velocity.x != 0 || m_velocity.y != 0);

    if (up && left)          m_currentRow = 2; 
	else if (up && right)    m_currentRow = 4; 
    else if (down && left)   m_currentRow = 1; 
    else if (down && right)  m_currentRow = 5; 
    else if (up)             m_currentRow = 3; 
    else if (down)           m_currentRow = 0; 
    else if (left)           m_currentRow = 1; 
    else if (right)          m_currentRow = 5; 
}

void Player::setGunDirectionForShooting(Gun::Direction dir)
{
	m_overrideGunDirection = dir;
	m_useOverrideDirection = true;
}

void Player::update(sf::Time dt)
{
	m_sprite.move(m_velocity * dt.asSeconds());

	// Animate if moving
	if (m_velocity.x != 0 || m_velocity.y != 0)
	{
		animate(dt);
	}
	else
	{
		m_currentFrame = 0;
		m_sprite.setTextureRect(sf::IntRect(0, m_currentRow * m_frameSize.y, m_frameSize.x, m_frameSize.y));
	}

	//footstep sounds
	if (m_isMoving)
	{
		m_footstepTimer += dt.asSeconds();
		
		if (m_footstepTimer >= m_footstepInterval)
		{
			m_footstepTimer = 0.f;
			m_footstepSound.play();
		}
	}
	else
	{
		// Reset timer and stop sound when not moving
		m_footstepTimer = 0.f;
		m_footstepSound.stop();
	}

	if (m_fireCooldown > 0.f)
		m_fireCooldown -= dt.asSeconds();

	if (m_useOverrideDirection && (m_gun.isAnimating() || m_gun.isFlashPlaying()))
	{
		// Keep locked to shooting direction
		m_gun.setDirection(m_overrideGunDirection);
	}
	else
	{
		m_useOverrideDirection = false;
		m_gun.setDirection(getGunDirection());
	}
	
	m_gun.setPosition(getPosition(), getSize());
	m_gun.update(dt);
}

sf::Vector2f Player::getSize() const
{
	return sf::Vector2f(m_sprite.getGlobalBounds().width, m_sprite.getGlobalBounds().height);
}

void Player::render(sf::RenderWindow& window)
{
	window.draw(m_sprite);
	m_gun.render(window);
}

void Player::takeDamage(int dmg)
{
	m_health -= dmg;
	if (m_health < 0)
	{
		m_health = 0;
	}
}

void Player::heal(int amount)
{
    m_health += amount;
    if (m_health > m_maxHealth)
    {
        m_health = m_maxHealth;
    }
}

void Player::animate(sf::Time dt)
{
	m_timeSinceLastFrame += dt.asSeconds();

	if (m_timeSinceLastFrame >= m_frameDuration) 
	{
		m_timeSinceLastFrame = 0.f;
		m_currentFrame = (m_currentFrame + 1) % m_frameCount;
	}

	m_sprite.setTextureRect(sf::IntRect(m_currentFrame * m_frameSize.x, m_currentRow * m_frameSize.y, m_frameSize.x, m_frameSize.y));
}

sf::Vector2i Player::getTilePos(float tileW, float tileH) const
{
	sf::Vector2f pos = m_sprite.getPosition();
	return {
		static_cast<int>(pos.x / tileW),
		static_cast<int>(pos.y / tileH)
	};
}

bool Player::canShoot() const 
{
	return m_fireCooldown <= 0.f && m_ammo > 0;
}

void Player::shoot() 
{
	m_fireCooldown = m_fireRate;
	m_ammo--;


	if (m_shootSound.getStatus() == sf::Sound::Playing)
	{
		m_shootSound.stop();
	}
	m_shootSound.play();

	m_gun.playShootAnimation();
}

Gun::Direction Player::getGunDirection() const
{
	switch (m_currentRow)
	{
	case 0:
		return Gun::Direction::Down;
	case 1:
		return Gun::Direction::Left;
	case 2:
		return Gun::Direction::Left;
	case 3:
		return Gun::Direction::Up;
	case 4:
		return Gun::Direction::Right;
	case 5:
		return Gun::Direction::Right;
	default:
		return Gun::Direction::Down;
	}
}

void Player::addAmmo(int amount)
{
	m_ammo = std::min(m_ammo + amount, m_maxAmmo);
}

void Player::playPickupSound()
{
	static bool loaded = false;
	if (!loaded)
	{
		if (!m_pickupBuffer.loadFromFile("ASSETS\\SOUNDS\\Pickup.wav"))
		{
			std::cout << "Failed to load pickup sound\n";
		}
		m_pickupSound.setBuffer(m_pickupBuffer);
		m_pickupSound.setVolume(50.f);
		loaded = true;
	}
	
	m_pickupSound.play();
}

void Player::applyDifficulty(const DifficultySettings& settings)
{
	m_health = settings.playerHealth;
	m_maxHealth = settings.playerHealth;
	m_ammo = settings.playerAmmo;
	m_maxAmmo = settings.playerAmmo;
	m_fireRate = settings.playerFireRate;
}


