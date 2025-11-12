/// <summary>
/// @author Moniks Nusi
/// @date 
/// </summary>

#include "Game.h"
#include <iostream>


Game::Game() :
	m_window{ sf::VideoMode{ 1200U, 1000U, 32U }, "SFML Game" },
	m_mapGenerator(8, 6, 100)
{
	m_mapGenerator.generate();

	for (int y = 0; y < 6; ++y)
		for (int x = 0; x < 8; ++x)
			if (m_mapGenerator.getRoom(x, y).color == sf::Color::Green)
				m_currentRoom = { x, y };

	m_cameraView = m_window.getDefaultView();
	m_cameraView.setCenter(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);
}

Game::~Game()
{
}

void Game::run()
{	
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	const float fps{ 60.0f };
	sf::Time timePerFrame = sf::seconds(1.0f / fps); // 60 fps
	while (m_window.isOpen())
	{
		processEvents(); // as many as possible
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents(); // at least 60 fps
			update(timePerFrame); //60 fps
		}
		render(); // as many as possible
	}
}

void Game::processEvents()
{
	sf::Event newEvent;
	while (m_window.pollEvent(newEvent))
	{
		if ( sf::Event::Closed == newEvent.type) // window message
		{
			m_exitGame = true;
		}
		if (sf::Event::KeyPressed == newEvent.type) //user pressed a key
		{
			processKeys(newEvent);
		}
	}
}

void Game::processKeys(sf::Event t_event)
{
	if (sf::Keyboard::Escape == t_event.key.code)
	{
		m_exitGame = true;
	}
}

void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();
	}
	m_player.hadnleInput();
	m_player.update(t_deltaTime);

	const auto& current = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	sf::Vector2f pos = m_player.getPosition();
	sf::Vector2f size = m_player.getSize();
	sf::Vector2f center = pos + size / 2.f;

	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;
	float margin = 40.f;

	// Handle active sliding transition
	if (m_transitionState == TransitionState::Sliding)
	{
		sf::Vector2f direction = m_slideTarget - m_slideOffset;
		float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

		if (distance > 1.f)
		{
			sf::Vector2f moveDir = direction / distance;
			m_slideOffset += moveDir * m_slideSpeed * t_deltaTime.asSeconds();

			// Clamp to prevent overshoot
			if (std::abs(m_slideOffset.x - m_slideTarget.x) < 2.f)
				m_slideOffset.x = m_slideTarget.x;
			if (std::abs(m_slideOffset.y - m_slideTarget.y) < 2.f)
				m_slideOffset.y = m_slideTarget.y;

			// Move camera
			m_cameraView.setCenter(windowW / 2.f + m_slideOffset.x,
				windowH / 2.f + m_slideOffset.y);
		}
		else
		{
			m_slideOffset = m_slideTarget;
			m_transitionState = TransitionState::None;
			m_currentRoom = m_nextRoom;

			// Reset player position to center of new room
			m_player.setPosition(windowW / 2.f - size.x / 2.f,
				windowH / 2.f - size.y / 2.f);

			// reset slide and camera to new room origin
			m_slideOffset = { 0.f, 0.f };
			m_cameraView.setCenter(windowW / 2.f, windowH / 2.f);
		}

		return;
	}

	// Detect when player walks into an exit
	if (m_transitionState == TransitionState::None)
	{
		sf::Vector2i newRoom = m_currentRoom;

		if (center.x > windowW - margin && current.exitRight)
			newRoom.x++;
		else if (center.x < margin && current.exitLeft)
			newRoom.x--;
		else if (center.y > windowH - margin && current.exitDown)
			newRoom.y++;
		else if (center.y < margin && current.exitUp)
			newRoom.y--;

		if (newRoom != m_currentRoom)
		{
			m_transitionState = TransitionState::Sliding;
			m_nextRoom = newRoom;

			// Calculate the world-space offset difference
			sf::Vector2f direction(
				(newRoom.x - m_currentRoom.x) * (float)windowW,
				(newRoom.y - m_currentRoom.y) * (float)windowH
			);

			m_slideStart = m_slideOffset;
			m_slideTarget = m_slideStart + direction;
		}
	}
}

void Game::render()
{
	m_window.setView(m_cameraView);
	m_window.clear(sf::Color(50, 50, 50));

	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	auto drawRoom = [&](const MapGenerator::Room& room, sf::Vector2f offset)
	{
		float tileW = (float)windowW / MapGenerator::Room::width;
		float tileH = (float)windowH / MapGenerator::Room::height;
		float tileSize = std::min(tileW, tileH);
		sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));

		for (int i = 0; i < room.height; ++i)
		{
			for (int j = 0; j < room.width; ++j)
			{
				tile.setPosition(offset.x + j * tileSize, offset.y + i * tileSize);

				// We’ll repeat the small texture across the large game tile.
				const int texSize = 32; // <-- change this to 16 if your texture is 16x16
				int repeatX = static_cast<int>(tileSize) / texSize;
				int repeatY = static_cast<int>(tileSize) / texSize;

				// avoid 0
				if (repeatX < 1) repeatX = 1;
				if (repeatY < 1) repeatY = 1;

				if (room.tiles[i][j] == 1) // wall
				{
					tile.setTexture(&m_mapGenerator.getWallTexture());
					tile.setTextureRect(sf::IntRect(0, 0, texSize * repeatX, texSize * repeatY));
				}
				else // floor
				{
					tile.setTexture(&m_mapGenerator.getFloorTexture());
					tile.setTextureRect(sf::IntRect(0, 0, texSize * repeatX, texSize * repeatY));
				}

				m_window.draw(tile);
			}
		}
	};

	// draw current room
	drawRoom(m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y), { 0.f, 0.f });

	// draw next room if sliding
	if (m_transitionState == TransitionState::Sliding)
	{
		sf::Vector2f offset(
			(m_nextRoom.x - m_currentRoom.x) * (float)windowW,
			(m_nextRoom.y - m_currentRoom.y) * (float)windowH
		);
		drawRoom(m_mapGenerator.getRoom(m_nextRoom.x, m_nextRoom.y), offset);
	}

	m_player.render(m_window);
	m_window.display();
}
