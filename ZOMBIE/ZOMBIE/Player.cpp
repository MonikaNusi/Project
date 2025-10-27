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
	m_sprite.setScale(2,2);

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
    
    if (up && left)          m_currentRow = 2; 
	else if (up && right)    m_currentRow = 4; 
    else if (down && left)   m_currentRow = 1; 
    else if (down && right)  m_currentRow = 5; 
    else if (up)             m_currentRow = 3; 
    else if (down)           m_currentRow = 0; 
    else if (left)           m_currentRow = 1; 
    else if (right)          m_currentRow = 5; 
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
}

sf::Vector2f Player::getSize() const
{
	return sf::Vector2f(m_sprite.getGlobalBounds().width, m_sprite.getGlobalBounds().height);
}

void Player::render(sf::RenderWindow& window)
{
	window.draw(m_sprite);
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
