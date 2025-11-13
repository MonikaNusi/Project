#pragma once
#include <SFML/Graphics.hpp>
class Player
{
public:
	Player();
	void hadnleInput();
	void update(sf::Time dt);
	void render(sf::RenderWindow& window);
	sf::Vector2f getSize() const;

	sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
	void setPosition(float x, float y) { m_sprite.setPosition(x, y); }

	sf::FloatRect getSpriteBounds() const { return m_sprite.getGlobalBounds(); }

private:
	sf::Sprite m_sprite;
	sf::Texture m_texture;

	sf::Vector2f m_velocity{ 0.f,0.f };
	float m_speed{ 200.f };


	int m_currentFrame{ 0 };
	const int m_frameCount{ 8 };
	sf::Vector2i m_frameSize{ 48,64 };
	float m_frameDuration{ 0.12f };
	float m_timeSinceLastFrame{ 0.f };
	int m_currentRow{ 0 };
	void animate(sf::Time dt);
	int getAnimationRow(bool up, bool down, bool left, bool right) const;
};

