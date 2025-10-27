/// <summary>
/// @author Monika Nusi
/// @date 
/// </summary>

#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "MapGenerator.h"

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

	sf::Vector2i m_nextRoom{ 0, 0 };
	sf::View m_cameraView;

	void processEvents();
	void processKeys(sf::Event t_event);
	void update(sf::Time t_deltaTime);
	void render();

	Player m_player;
	MapGenerator m_mapGenerator;
	sf::Vector2i m_currentRoom{ 0, 0 };

	sf::RenderWindow m_window; // main SFML window
	bool m_exitGame{ false }; // control exiting game

};
