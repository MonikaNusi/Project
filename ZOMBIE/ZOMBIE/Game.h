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
#include <map>
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
	void saveCurrentRoomState();
	void loadRoomState();

	Player m_player;
	MapGenerator m_mapGenerator;
	std::vector<std::vector<bool>> m_visitedRooms;
	sf::Vector2i m_currentRoom{ 0, 0 };
	sf::Vector2f m_lastPlayerPos;

	sf::FloatRect m_debugPlayerBox;

	std::vector<Zombie> m_zombies;
	sf::Texture m_zombieTexture;

	// Store zombies per room using room coordinates as key
	std::map<std::pair<int, int>, std::vector<Zombie>> m_roomZombies;

	sf::RenderWindow m_window; // main SFML window
	bool m_exitGame{ false }; // control exiting game

	// Key / unlock system
	struct Key
	{
		sf::Vector2f pos;
		bool picked{ false };
		sf::Sprite sprite;           
		int currentFrame{ 0 };    
		float frameTimer{ 0.f };

		Key() = default;
		Key(sf::Vector2f p)
			: pos(p)
		{
			sprite.setOrigin({ 9.f, 9.f });
			sprite.setPosition(p);
		}
	};

	std::vector<Key> m_keys;
	bool m_playerHasKey{ false };

	sf::Texture m_keyTexture;
	const int m_keyFrameCount{ 27 };
	const float m_keyFrameWidth{ 21.f };
	const float m_keyFrameTime{ 0.05f };

	// Store keys per room
	std::map<std::pair<int, int>, std::vector<Key>> m_roomKeys;

	// unlocking progress
	float m_unlockHoldTimer{ 0.f };
	const float m_unlockHoldRequired{ 5.f };
	bool m_isUnlocking{ false };
	sf::Vector2i m_unlockTargetRoom{ -1, -1 };
	int m_unlockTargetDirX{ 0 };
	int m_unlockTargetDirY{ 0 };


	bool m_debugNearLockedDoor{ false };
};
