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

};
