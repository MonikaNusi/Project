/// <summary>
/// @author Monika Nusi
/// @date 
/// </summary>

#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "MapGenerator.h"
#include "Zombie.h"
#include <vector>
#include "Bullet.h"

class Game
{
public:
	Game();
	~Game();
	void run();

private:

	enum class TransitionState { None,Sliding };
	TransitionState m_transitionState{ TransitionState::None };
	
	sf::Vector2f m_slideStart;   // starting camera offset
	sf::Vector2f m_slideTarget;  // target camera offset
	sf::Vector2f m_slideOffset;  // current offset during slide
	sf::Vector2f m_nextOffset;
	float m_slideSpeed = 800.f;

	sf::Vector2f m_playerSlideStartPos;

	sf::Vector2i m_nextRoom{ 0, 0 };
	sf::View m_cameraView;

	sf::Font m_debugFont;

	std::vector<Bullet> m_bullets;

	sf::Font m_uiFont;
	sf::Text m_ammoText;

	sf::RectangleShape m_healthBarBack;
	sf::RectangleShape m_healthBarFront;

	bool m_showAIDebug = false;

	void processEvents();
	void processKeys(sf::Event t_event);
	void update(sf::Time t_deltaTime);
	void render();
	void drawMiniMap();
	bool isCollidingWithWall(const sf::FloatRect& playerBox);
	sf::Vector2f findSafeSpawn(const MapGenerator::Room& room);
	sf::Vector2f getDoorSpawn(const MapGenerator::Room& room,
		int dirX, int dirY);
	void spawnZombiesForRoom();

	Player m_player;
	MapGenerator m_mapGenerator;
	std::vector<std::vector<bool>> m_visitedRooms;
	sf::Vector2i m_currentRoom{ 0, 0 };
	sf::Vector2f m_lastPlayerPos;

	sf::FloatRect m_debugPlayerBox;

	std::vector<Zombie> m_zombies;
	sf::Texture m_zombieTexture;

	sf::RenderWindow m_window; // main SFML window
	bool m_exitGame{ false }; // control exiting game

	// Key / unlock system
	struct Key
	{
		sf::Vector2f pos;
		bool picked{ false };
		sf::RectangleShape shape;
		Key() = default;
		Key(sf::Vector2f p)
			: pos(p)
		{
			shape.setSize({ 18.f, 18.f });
			shape.setOrigin({ 9.f, 9.f });
			shape.setPosition(p);
			shape.setFillColor(sf::Color::Yellow);
		}
	};

	std::vector<Key> m_keys;
	bool m_playerHasKey{ false };

	// unlocking progress
	float m_unlockHoldTimer{ 0.f };
	const float m_unlockHoldRequired{ 5.f };
	bool m_isUnlocking{ false };
	sf::Vector2i m_unlockTargetRoom{ -1, -1 };
	int m_unlockTargetDirX{ 0 };
	int m_unlockTargetDirY{ 0 };

	// debug / UX: show when player is near a locked door (set in update, read in render)
	bool m_debugNearLockedDoor{ false };
};
