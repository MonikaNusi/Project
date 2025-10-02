/// <summary>
/// @author Monika Nusi
/// @date 
/// </summary>

#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"

class Game
{
public:
	Game();
	~Game();
	void run();

private:

	void processEvents();
	void processKeys(sf::Event t_event);
	void update(sf::Time t_deltaTime);
	void render();

	Player m_player;

	sf::RenderWindow m_window; // main SFML window
	bool m_exitGame{ false }; // control exiting game

};
