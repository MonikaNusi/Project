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

	const auto& newRoomObj = m_mapGenerator.getRoom(m_nextRoom.x, m_nextRoom.y);

	int dirX = m_nextRoom.x - m_currentRoom.x;  // -1, 0, or 1
	int dirY = m_nextRoom.y - m_currentRoom.y;  // -1, 0, or 1

	sf::Vector2f doorPos = getDoorSpawn(newRoomObj, dirX, dirY);
	m_player.setPosition(doorPos.x, doorPos.y);


	m_cameraView = m_window.getDefaultView();
	m_cameraView.setCenter(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);
	//m_lastPlayerPos = m_player.getPosition();

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

	sf::Vector2f oldPos = m_player.getPosition();

	if (m_transitionState != TransitionState::Sliding)
	{
		m_player.hadnleInput();
		m_player.update(t_deltaTime);
		std::cout<<oldPos.y<<std::endl;
	}
	sf::FloatRect spriteBounds = m_player.getSpriteBounds();


	// Tweakable hitbox % values
	float hbWidthPercent = 0.30f;
	float hbHeightPercent = 0.12f;
	float yOffsetPercent = 0.28f;  // lift hitbox upward

	float hbWidth = spriteBounds.width * hbWidthPercent;
	float hbHeight = spriteBounds.height * hbHeightPercent;
	float yOffset = spriteBounds.height * yOffsetPercent;

	sf::FloatRect playerBox(
		spriteBounds.left + (spriteBounds.width - hbWidth) * 0.5f,
		spriteBounds.top + spriteBounds.height - hbHeight - yOffset,
		hbWidth,
		hbHeight
	);

	m_debugPlayerBox = playerBox;

	//If this new position collides with wall undo movement
	if (isCollidingWithWall(playerBox))
	{
		m_player.setPosition(oldPos.x, oldPos.y);

		// update bounds after resetting
		spriteBounds = m_player.getSpriteBounds();
	}



	sf::Vector2f pos = m_player.getPosition();
	sf::Vector2f size = m_player.getSize();
	sf::Vector2f center = pos + size / 2.f;

	const auto& current = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
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
			int oldX = m_currentRoom.x;
			int oldY = m_currentRoom.y;
			m_currentRoom = m_nextRoom;

			//safe space in the next room

			const auto& nextRoom = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

			int dirX = m_currentRoom.x - oldX;
			int dirY = m_currentRoom.y - oldY;

			sf::Vector2f doorPos = getDoorSpawn(nextRoom, dirX, dirY);
			m_player.setPosition(doorPos.x, doorPos.y);

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

sf::Vector2f Game::findSafeSpawn(const MapGenerator::Room& room)
{
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	//Try the center first
	int cx = room.width / 2;
	int cy = room.height / 2;

	if (room.tiles[cy][cx] == 0) //FLOOR
	{
		return { cx * tileW, cy * tileH };
	}

	//Otherwise search for ANY nearby floor tile
	for (int y = 1; y < room.height - 1; ++y)
	{
		for (int x = 1; x < room.width - 1; ++x)
		{
			if (room.tiles[y][x] == 0)
			{
				return { x * tileW, y * tileH };
			}
		}
	}

	return { windowW / 2.f, windowH / 2.f };
}

sf::Vector2f Game::getDoorSpawn(const MapGenerator::Room& room,
	int dirX, int dirY)
{
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	int midX = room.width / 2;
	int midY = room.height / 2;

	// Coming from left - spawn at left door
	if (dirX == 1)     return { 1 * tileW,       midY * tileH };
	// Coming from right - spawn at right door
	if (dirX == -1)    return { (room.width - 2) * tileW, midY * tileH };
	// Coming from top - spawn at top door
	if (dirY == 1)     return { midX * tileW,    1 * tileH };
	// Coming from bottom - spawn at bottom door
	if (dirY == -1)    return { midX * tileW,    (room.height - 2) * tileH };

	return { midX * tileW, midY * tileH };
}

bool Game::isCollidingWithWall(const sf::FloatRect& playerBox)
{
	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	// Find which tiles the player is overlapping
	int leftTile = playerBox.left / tileW;
	int rightTile = (playerBox.left + playerBox.width) / tileW;
	int topTile = playerBox.top / tileH;
	int bottomTile = (playerBox.top + playerBox.height) / tileH;

	// Clamp bounds
	leftTile = std::max(0, std::min(room.width - 1, leftTile));
	rightTile = std::max(0, std::min(room.width - 1, rightTile));
	topTile = std::max(0, std::min(room.height - 1, topTile));
	bottomTile = std::max(0, std::min(room.height - 1, bottomTile));

	// Check any wall tile
	for (int y = topTile; y <= bottomTile; ++y)
	{
		for (int x = leftTile; x <= rightTile; ++x)
		{
			if (room.tiles[y][x] == 1) // wall tile
			{
				return true;
			}
		}
	}

	return false;
}

void Game::render()
{
	m_window.setView(m_cameraView);
	m_window.clear(sf::Color(50, 50, 50));

	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	auto drawRoom = [&](const MapGenerator::Room& room, sf::Vector2f offset)
	{

		float tileW = static_cast<float>(windowW) / room.width;
		float tileH = static_cast<float>(windowH) / room.height;
		sf::RectangleShape tile(sf::Vector2f(tileW, tileH));
		
		for (int i = 0; i < room.height; ++i)
		{
			for (int j = 0; j < room.width; ++j)
			{

				tile.setPosition(offset.x + j * tileW, offset.y + i * tileH);

				const int texSize = 16;
				int repeatX = static_cast<int>(std::ceil(tileW / texSize));
				int repeatY = static_cast<int>(std::ceil(tileH / texSize));
			
				if (repeatX < 1) repeatX = 1;
				if (repeatY < 1) repeatY = 1;

				if (room.tiles[i][j] == 1) // wall
				{
					//tile.setTexture(&m_mapGenerator.getWallTexture());
					//tile.setTextureRect(sf::IntRect(0, 0, texSize * repeatX, texSize * repeatY));
					tile.setFillColor(sf::Color(40, 40, 40));
				}
				else // floor
				{
					tile.setFillColor(sf::Color(200, 200, 200));
					//tile.setTexture(&m_mapGenerator.getFloorTexture());
					//tile.setTextureRect(sf::IntRect(0, 0, texSize * repeatX, texSize * repeatY));
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

	//m_mapGenerator.render(m_window);
	m_player.render(m_window);
		
	sf::RectangleShape hb;
	hb.setPosition(m_debugPlayerBox.left, m_debugPlayerBox.top);
	hb.setSize({ m_debugPlayerBox.width, m_debugPlayerBox.height });
	hb.setFillColor(sf::Color(255, 0, 0, 120));
	m_window.draw(hb);

	m_window.setView(m_window.getDefaultView());

	drawMiniMap();

	m_window.display();
}

void Game::drawMiniMap()
{
	const int mapWidth = 8;
	const int mapHeight = 6;

	const float cellSize = 18.f;
	const float spacing = 2.f;
	const float padding = 10.f;

	//minimap pixel size
	float mapPixelW = mapWidth * (cellSize + spacing);
	float mapPixelH = mapHeight * (cellSize + spacing);

	sf::RectangleShape frame;
	frame.setSize(sf::Vector2f(mapPixelW + padding * 2,
		mapPixelH + padding * 2));
	frame.setPosition(20.f, 20.f);
	frame.setFillColor(sf::Color(20, 20, 20, 180));
	frame.setOutlineThickness(3.f);
	frame.setOutlineColor(sf::Color(200, 200, 200, 180));

	m_window.draw(frame);

	// Starting position inside frame
	float startX = frame.getPosition().x + padding;
	float startY = frame.getPosition().y + padding;

	sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));

	// Draw rooms
	for (int y = 0; y < mapHeight; ++y)
	{
		for (int x = 0; x < mapWidth; ++x)
		{
			const auto& room = m_mapGenerator.getRoom(x, y);

			// inactive = dark gray
			if (!room.active)
				cell.setFillColor(sf::Color(60, 60, 60));
			else
				cell.setFillColor(sf::Color(150, 150, 150));

			// start room = green
			if (room.type == MapGenerator::Room::RoomType::Start)
				cell.setFillColor(sf::Color::Green);

			// boss room = red
			if (room.type == MapGenerator::Room::RoomType::Boss)
				cell.setFillColor(sf::Color::Red);

			// current room highlight = yellow
			if (x == m_currentRoom.x && y == m_currentRoom.y)
				cell.setFillColor(sf::Color(255, 230, 50));

			// Set position inside minimap frame
			cell.setPosition(startX + x * (cellSize + spacing),
				startY + y * (cellSize + spacing));

			m_window.draw(cell);
		}
	}
}
