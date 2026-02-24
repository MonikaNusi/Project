/// <summary>
/// @author Moniks Nusi
/// @date 
/// </summary>

#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

std::string zombieStateToString(Zombie::State state);


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

	m_visitedRooms.resize(6, std::vector<bool>(8, false));
	m_visitedRooms[m_currentRoom.y][m_currentRoom.x] = true; //start rooms visited


	spawnZombiesForRoom();
	

	if (!m_debugFont.loadFromFile("ASSETS/FONTS/ariblk.ttf"))
	{
		std::cout << "Failed to load debug font\n";
	}

	m_uiFont.loadFromFile("ASSETS/FONTS/ariblk.ttf");
	m_ammoText.setFont(m_uiFont); 
	m_ammoText.setCharacterSize(24);
	m_ammoText.setFillColor(sf::Color::White);
	m_ammoText.setPosition(950.f, 20.f);

	m_healthBarBack.setSize({ 200.f, 20.f });
	m_healthBarBack.setFillColor(sf::Color(80, 0, 0));
	m_healthBarBack.setPosition(700.f, 20.f);

	m_healthBarFront.setSize({ 200.f, 20.f });
	m_healthBarFront.setFillColor(sf::Color(200, 0, 0));
	m_healthBarFront.setPosition(700.f, 20.f);

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
		if (newEvent.type == sf::Event::MouseButtonPressed &&
			newEvent.mouseButton.button == sf::Mouse::Left)
		{
			if (m_player.canShoot())
			{
				// shoot 
				sf::Vector2f playerPos = m_player.getPosition();
				sf::Vector2f playerSize = m_player.getSize();
				sf::Vector2f playerCenter = playerPos + playerSize / 2.f;

				// mouse position
				sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
				sf::Vector2f mouseWorld = m_window.mapPixelToCoords(mousePixel, m_cameraView);
				sf::Vector2f dir = mouseWorld - playerCenter;

				m_player.shoot(); // reduces ammo, sets cooldown
				m_bullets.emplace_back(playerCenter, dir);
			}
		}
	}
}

void Game::processKeys(sf::Event t_event)
{
	if (sf::Keyboard::Escape == t_event.key.code)
	{
		m_exitGame = true;
	}
	if (sf::Keyboard::Q == t_event.key.code)
	{
		m_showAIDebug = !m_showAIDebug;
	}
}

void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();
	}

	m_ammoText.setString("Ammo: " + std::to_string(m_player.getAmmo()));

	sf::Vector2f oldPos = m_player.getPosition();

	sf::Vector2f playerCenter = m_player.getPosition() + m_player.getSize() / 2.f;

	if (m_transitionState != TransitionState::Sliding)
	{
		m_player.hadnleInput();
		m_player.update(t_deltaTime);
		//std::cout<<oldPos.y<<std::endl;
	}

	if (m_transitionState != TransitionState::Sliding)
	{
		std::vector<Zombie*> zombiePtrs;
		for (auto& other : m_zombies)
			zombiePtrs.push_back(&other);

		for (auto& z : m_zombies)
		{
			sf::Vector2f oldPos = z.getPosition();

			const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

			float tileW = (float)m_window.getSize().x / room.width;
			float tileH = (float)m_window.getSize().y / room.height;

			z.update(
				t_deltaTime,
				playerCenter,  
				room.tiles,
				tileW,
				tileH,
				zombiePtrs
			);

			// Zombie attack
			if (z.getState() == Zombie::State::Attack)
			{
				sf::FloatRect playerHitbox = m_debugPlayerBox;
				int dmg = z.tryDealDamage(playerHitbox);
				if (dmg > 0)
				{
					m_player.takeDamage(dmg);
				}
			}

			if (m_player.getHealth() <= 0)
			{
				std::cout << "GAME OVER\n";
				m_window.close();
			}



			if (isCollidingWithWall(z.getHitbox()))
			{

				sf::Vector2f newPos = z.getPosition();

	
				z.setPosition(newPos.x, oldPos.y);
				if (isCollidingWithWall(z.getHitbox()))
				{
				
					z.setPosition(oldPos.x, newPos.y);
					if (isCollidingWithWall(z.getHitbox()))
					{

						z.setPosition(oldPos.x, oldPos.y);
					}
				}
			}
		}
	}

	// Handle dead zombies and spawn dropped keys
	for (size_t i = 0; i < m_zombies.size(); )
	{
		if (m_zombies[i].isDead())
		{
			if (m_zombies[i].hasKey())
			{
				// spawn key at zombie position
				m_keys.emplace_back(m_zombies[i].getPosition());
			}

			m_zombies.erase(m_zombies.begin() + i);
		}
		else
		{
			++i;
		}
	}

	float healthPercent = (float)m_player.getHealth() / m_player.getMaxHealth();
	m_healthBarFront.setSize({ 200.f * healthPercent, 20.f });



	sf::FloatRect spriteBounds = m_player.getSpriteBounds();




	float hbWidthPercent = 0.30f;
	float hbHeightPercent = 0.12f;
	float yOffsetPercent = 0.28f;  // lift hitbox upward

	float hbWidth = spriteBounds.width * hbWidthPercent;
	float hbHeight = spriteBounds.height * hbHeightPercent;
	float yOffset = spriteBounds.height * yOffsetPercent;

	//hitbox
	sf::FloatRect playerBox(
		spriteBounds.left + (spriteBounds.width - hbWidth) * 0.5f, 
		spriteBounds.top + spriteBounds.height - hbHeight - yOffset, //positioned near feet
		hbWidth,
		hbHeight
	);

	//debugging
	m_debugPlayerBox = playerBox;

	//If this new position collides with wall undo movement
	if (isCollidingWithWall(playerBox))
	{
		m_player.setPosition(oldPos.x, oldPos.y);

		// update bounds after resetting
		spriteBounds = m_player.getSpriteBounds();
	}


	for (auto& b : m_bullets) 
	{
		b.update(t_deltaTime);
	}

	m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
		[](const Bullet& b) { return !b.isAlive(); }), m_bullets.end());

	for (auto& b : m_bullets) 
	{
		if (!b.isAlive()) continue;

		for (auto& z : m_zombies) 
		{
			if (z.isDead()) continue;

			if (b.getBounds().intersects(z.getHitbox())) 
			{
				z.takeDamage(25);
				b.kill();
			}
		}

	}

	// handle key pickup by player
	for (auto& key : m_keys)
	{
		if (key.picked) continue;
		sf::FloatRect keyRect(key.pos.x - 9.f, key.pos.y - 9.f, 18.f, 18.f);
		if (keyRect.intersects(playerBox))
		{
			key.picked = true;
			m_playerHasKey = true;
			std::cout << "Picked up key\n";
		}
	}

	// remove picked keys from world vector
	m_keys.erase(std::remove_if(m_keys.begin(), m_keys.end(), [](const Key& k) { return k.picked; }), m_keys.end());


	sf::Vector2f pos = m_player.getPosition();
	sf::Vector2f size = m_player.getSize();
	sf::Vector2f center = pos + size / 2.f;

	const auto& current = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;
	float margin = 40.f;

	// tile sizes and door mid positions to check locked status
	float tileW = static_cast<float>(windowW) / current.width;
	float tileH = static_cast<float>(windowH) / current.height;
	int midX = current.width / 2;
	int midY = current.height / 2;

	//check nearby locked door tile and handle E hold
	bool nearLockedDoor = false;
	int doorRoomX = -1, doorRoomY = -1;
	int doorDirX = 0, doorDirY = 0;
	sf::Vector2f doorTileCenter;

	//checks 3 locked tiles for a door and detects proximity
	auto checkDoor = [&](int tx, int ty, int dx, int dy) -> bool
		{
			for (int o = -1; o <= 1; ++o)
			{
				int cx = tx + (dx != 0 ? 0 : o);
				int cy = ty + (dy != 0 ? 0 : o);

				if (cx < 0 || cy < 0 || cx >= current.width || cy >= current.height)
					continue;

				if (current.tiles[cy][cx] == 2) // locked tile
				{
					float px = (cx + 0.5f) * tileW;
					float py = (cy + 0.5f) * tileH;

					float dxp = px - center.x;
					float dyp = py - center.y;
					float dist = std::sqrt(dxp * dxp + dyp * dyp);

					// Increased detection radius
					if (dist < 80.f)
					{
						nearLockedDoor = true;
						doorRoomX = m_currentRoom.x;
						doorRoomY = m_currentRoom.y;
						doorDirX = dx;
						doorDirY = dy;
						doorTileCenter = { px, py };
						return true;
					}
				}
			}
			return false;
		};

	// Check all four door directions
	if (current.exitRight)
	{
		int tx = current.width - 1;
		int ty = midY;
		if (checkDoor(tx, ty, 1, 0))
			goto foundDoor;
	}

	if (current.exitLeft)
	{
		int tx = 0;
		int ty = midY;
		if (checkDoor(tx, ty, -1, 0))
			goto foundDoor;
	}

	if (current.exitDown)
	{
		int tx = midX;
		int ty = current.height - 1;
		if (checkDoor(tx, ty, 0, 1))
			goto foundDoor;
	}

	if (current.exitUp)
	{
		int tx = midX;
		int ty = 0;
		if (checkDoor(tx, ty, 0, -1))
			goto foundDoor;
	}

foundDoor:
	m_debugNearLockedDoor = nearLockedDoor;

	//unlocking logic

	if (nearLockedDoor && m_playerHasKey)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			if (!m_isUnlocking)
			{
				// Start unlocking
				m_isUnlocking = true;
				m_unlockTargetRoom = { doorRoomX, doorRoomY };
				m_unlockTargetDirX = doorDirX;
				m_unlockTargetDirY = doorDirY;
				m_unlockHoldTimer = 0.f;
				std::cout << "Started unlocking door at (" << doorDirX << "," << doorDirY << ")\n";
			}
			else
			{
				// Continue holding - check if same door
				if (m_unlockTargetRoom.x == doorRoomX &&
					m_unlockTargetRoom.y == doorRoomY &&
					m_unlockTargetDirX == doorDirX &&
					m_unlockTargetDirY == doorDirY)
				{
					m_unlockHoldTimer += t_deltaTime.asSeconds();

					if (m_unlockHoldTimer >= m_unlockHoldRequired)
					{
						//unlock the door
						int rx = doorRoomX;
						int ry = doorRoomY;
						int nx = rx + doorDirX;
						int ny = ry + doorDirY;

						// Clear door tiles in both current and next room
						auto clearDoorTiles = [&](int roomX, int roomY, int dx, int dy)
							{
								if (roomX < 0 || roomY < 0 || roomX >= 8 || roomY >= 6)
									return;

								int mid = MapGenerator::Room::width / 2;

								if (dx == 1) // right door
								{
									for (int dy2 = -1; dy2 <= 1; ++dy2)
										m_mapGenerator.setTile(roomX, roomY,
											MapGenerator::Room::width - 1,
											mid + dy2, 0);
								}
								else if (dx == -1) // left door
								{
									for (int dy2 = -1; dy2 <= 1; ++dy2)
										m_mapGenerator.setTile(roomX, roomY,
											0, mid + dy2, 0);
								}
								else if (dy == 1) // down door
								{
									for (int dx2 = -1; dx2 <= 1; ++dx2)
										m_mapGenerator.setTile(roomX, roomY,
											mid + dx2,
											MapGenerator::Room::height - 1, 0);
								}
								else if (dy == -1) // up door
								{
									for (int dx2 = -1; dx2 <= 1; ++dx2)
										m_mapGenerator.setTile(roomX, roomY,
											mid + dx2, 0, 0);
								}
							};

						clearDoorTiles(rx, ry, doorDirX, doorDirY);
						clearDoorTiles(nx, ny, -doorDirX, -doorDirY);

						m_playerHasKey = false;
						m_unlockHoldTimer = 0.f;
						m_isUnlocking = false;
						m_unlockTargetRoom = { -1, -1 };

						std::cout << "Door unlocked successfully!\n";
					}
				}
				else
				{
					// Switched to different door - restart
					m_unlockHoldTimer = 0.f;
					m_isUnlocking = true;
					m_unlockTargetRoom = { doorRoomX, doorRoomY };
					m_unlockTargetDirX = doorDirX;
					m_unlockTargetDirY = doorDirY;
					std::cout << "Switched unlocking target\n";
				}
			}
		}
		else
		{
			// E key released - cancel unlock
			if (m_isUnlocking)
				std::cout << "Unlock cancelled (key released)\n";

			m_unlockHoldTimer = 0.f;
			m_isUnlocking = false;
			m_unlockTargetRoom = { -1, -1 };
		}
	}
	else
	{
		// Not near door or no key - cancel unlock
		if (m_isUnlocking)
			std::cout << "Unlock cancelled (moved away or no key)\n";

		m_unlockHoldTimer = 0.f;
		m_isUnlocking = false;
		m_unlockTargetRoom = { -1, -1 };
	}


	// sliding / room transition logic
	// Handle active sliding transition
	if (m_transitionState == TransitionState::Sliding)
	{

		//how far we still need to slide
		sf::Vector2f direction = m_slideTarget - m_slideOffset;
		float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

		if (distance > 1.f) //continue slideing
		{
			sf::Vector2f moveDir = direction / distance;  //normalize direction
			m_slideOffset += moveDir * m_slideSpeed * t_deltaTime.asSeconds(); //move camera

			// Clamp x movement so it stops cleanly
			if (std::abs(m_slideOffset.x - m_slideTarget.x) < 2.f)
				m_slideOffset.x = m_slideTarget.x;
			//clamp y movement so it stops cleanly
			if (std::abs(m_slideOffset.y - m_slideTarget.y) < 2.f)
				m_slideOffset.y = m_slideTarget.y;

			// Move camera
			m_cameraView.setCenter(windowW / 2.f + m_slideOffset.x,
				windowH / 2.f + m_slideOffset.y);
		}
		else  //sliding is finished
		{
			m_slideOffset = m_slideTarget;  //snap offset to the final position
			m_transitionState = TransitionState::None;  //end transition
			int oldX = m_currentRoom.x;
			int oldY = m_currentRoom.y;
			m_currentRoom = m_nextRoom;  //switch to the next room

			for (auto& z : m_zombies)
				z.setFrozen(true);

			spawnZombiesForRoom();


			m_visitedRooms[m_currentRoom.y][m_currentRoom.x] = true;  //mark room as visited

			const auto& nextRoom = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

			int dirX = m_currentRoom.x - oldX;
			int dirY = m_currentRoom.y - oldY;

			sf::Vector2f doorPos = getDoorSpawn(nextRoom, dirX, dirY);
			m_player.setPosition(doorPos.x, doorPos.y); //place the player at the correct door

			


			//read slide offst and camera
			m_slideOffset = { 0.f, 0.f };
			m_cameraView.setCenter(windowW / 2.f, windowH / 2.f);
		}

		return;
	}

	// Detect when player walks into an exit
	if (m_transitionState == TransitionState::None)
	{
		sf::Vector2i newRoom = m_currentRoom; //start with the current room

		//player is at right edge and this room has a right exit AND the door tile is NOT locked
		if (center.x > windowW - margin && current.exitRight)
		{
			// Check the border tile for locked status (locked tile value == 2)
			if (current.tiles[midY][current.width - 1] == 0)
				newRoom.x++;
		}
		//player is at left edge and this room has a left exit AND not locked
		else if (center.x < margin && current.exitLeft)
		{
			if (current.tiles[midY][0] == 0)
				newRoom.x--;
		}
		//player is at bottom edge and this room has a down exit AND not locked
		else if (center.y > windowH - margin && current.exitDown)
		{
			if (current.tiles[current.height - 1][midX] == 0)
				newRoom.y++;
		}
		//player is at top edge and this room has an up exit AND not locked
		else if (center.y < margin && current.exitUp)
		{
			if (current.tiles[0][midX] == 0)
				newRoom.y--;
		}

		//if newRoom changed player endered the door
		if (newRoom != m_currentRoom)
		{
			m_transitionState = TransitionState::Sliding;
			m_nextRoom = newRoom;

			// Calculate how far the camera needs to slide
			sf::Vector2f direction(
				(newRoom.x - m_currentRoom.x) * (float)windowW,
				(newRoom.y - m_currentRoom.y) * (float)windowH
			);

			m_slideStart = m_slideOffset;
			m_slideTarget = m_slideStart + direction;
		}
	}
}

void Game::spawnZombiesForRoom()
{
	m_zombies.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

	// Decide count based on room type
	int count = 0;

	switch (room.type)
	{
	case MapGenerator::Room::RoomType::Normal:
		count = 5;
		break;
	case MapGenerator::Room::RoomType::Trap:
		count = 10;
		break;
	case MapGenerator::Room::RoomType::Treasure:
		count = 1;
		break;
	case MapGenerator::Room::RoomType::Boss:
		count = 0; // boss later
		break;
	default:
		count = 1;
		break;
	}

	for (int i = 0; i < count; ++i)
	{
		Zombie z(m_zombieTexture);

		sf::Vector2f spawn = findSafeSpawn(room);
		z.reset(spawn);

		m_zombies.push_back(z);
	}

	// randomly assign one zombie to carry the key
	if (!m_zombies.empty())
	{
		int idx = std::rand() % static_cast<int>(m_zombies.size());
		m_zombies[idx].setHasKey(true);
		std::cout << "Assigned key to zombie " << idx << "\n";
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
		return {
			cx * tileW + tileW * 0.5f,
			cy * tileH + tileH * 0.5f
		};
	}

	//Otherwise search for any nearby floor tile
	for (int y = 1; y < room.height - 1; ++y)
	{
		for (int x = 1; x < room.width - 1; ++x)
		{
			if (room.tiles[y][x] == 0)
			{
				return {
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				};
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
	//get the current room where the player is in
	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	//calculate tile size in pixels
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

	// Check any wall tile (treat locked tiles (2) as wall as well)
	for (int y = topTile; y <= bottomTile; ++y)
	{
		for (int x = leftTile; x <= rightTile; ++x)
		{
			if (room.tiles[y][x] == 1 || room.tiles[y][x] == 2) // wall or locked door
			{
				return true; //collision detected
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
				else if (room.tiles[i][j] == 2) // locked door
				{
					// Draw locked doors in a different color so player can see them
					tile.setFillColor(sf::Color(100, 60, 180));
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

	for (auto& z : m_zombies)
	{
		z.render(m_window);

		/*sf::Text stateText;
		stateText.setFont(m_debugFont);
		stateText.setCharacterSize(14);
		stateText.setFillColor(sf::Color::White);

		stateText.setString(zombieStateToString(z.getState()));

		sf::Vector2f pos = z.getPosition();
		stateText.setPosition(pos.x - 20.f, pos.y - 60.f);

		m_window.draw(stateText);*/

		if (m_showAIDebug)
		{
			// Alert radius
			sf::CircleShape alertCircle;
			alertCircle.setRadius(z.getAlertRadius());
			alertCircle.setOrigin(z.getAlertRadius(), z.getAlertRadius());
			alertCircle.setPosition(z.getPosition());
			alertCircle.setFillColor(sf::Color::Transparent);
			alertCircle.setOutlineThickness(1.f);
			alertCircle.setOutlineColor(sf::Color(255, 200, 0, 120));
			m_window.draw(alertCircle);

			// Chase range
			sf::CircleShape chaseCircle;
			chaseCircle.setRadius(z.getChaseRange());
			chaseCircle.setOrigin(z.getChaseRange(), z.getChaseRange());
			chaseCircle.setPosition(z.getPosition());
			chaseCircle.setFillColor(sf::Color::Transparent);
			chaseCircle.setOutlineThickness(1.f);
			chaseCircle.setOutlineColor(sf::Color(0, 150, 255, 120));
			m_window.draw(chaseCircle);

			// Last known player position
			if (z.isAlerted())
			{
				sf::Vector2f p = z.getLastKnownPlayerPos();

				sf::RectangleShape crossH({ 10.f, 2.f });
				sf::RectangleShape crossV({ 2.f, 10.f });

				crossH.setOrigin(5.f, 1.f);
				crossV.setOrigin(1.f, 5.f);

				crossH.setPosition(p);
				crossV.setPosition(p);

				crossH.setFillColor(sf::Color::Red);
				crossV.setFillColor(sf::Color::Red);

				m_window.draw(crossH);
				m_window.draw(crossV);
			}
		}



		sf::RectangleShape hb;
		auto zb = z.getHitbox();
		hb.setPosition(zb.left, zb.top);
		hb.setSize({ zb.width, zb.height });
		hb.setFillColor(sf::Color(0, 255, 0, 120));
		m_window.draw(hb);
	}

	// draw dropped keys
	for (const auto& key : m_keys)
	{
		if (!key.picked)
		{
			m_window.draw(key.shape);
		}
	}

	for (const auto& b : m_bullets)
	{
		b.render(m_window);
	}

			
	sf::RectangleShape hb;
	hb.setPosition(m_debugPlayerBox.left, m_debugPlayerBox.top);
	hb.setSize({ m_debugPlayerBox.width, m_debugPlayerBox.height });
	hb.setFillColor(sf::Color(255, 0, 0, 120));
	m_window.draw(hb);


	// draw UI in screen space
	m_window.setView(m_window.getDefaultView());

	m_window.draw(m_ammoText);

	m_window.draw(m_healthBarBack);
	m_window.draw(m_healthBarFront);
	m_window.draw(m_ammoText);

	// draw key indicator
	if (m_playerHasKey)
	{
		sf::Text t("Key: YES", m_uiFont, 20);
		t.setPosition(700.f, 50.f);
		t.setFillColor(sf::Color::Yellow);
		m_window.draw(t);
	}
	else
	{
		sf::Text t("Key: NO", m_uiFont, 20);
		t.setPosition(700.f, 50.f);
		t.setFillColor(sf::Color::White);
		m_window.draw(t);
	}

	// draw unlock progress if active
	

	// draw unlock hint if near door and has key
	m_window.setView(m_window.getDefaultView());
	if (m_debugNearLockedDoor && m_playerHasKey)
	{
		sf::Text hint("Hold E to unlock", m_uiFont, 18);
		hint.setPosition(700.f, 110.f);
		hint.setFillColor(sf::Color::White);
		m_window.draw(hint);

		if (m_isUnlocking)
		{
			std::string s = "Unlocking: " + std::to_string(static_cast<int>(m_unlockHoldTimer)) + " / " + std::to_string(static_cast<int>(m_unlockHoldRequired));
			sf::Text tx(s, m_uiFont, 16);
			tx.setPosition(700.f, 135.f);
			tx.setFillColor(sf::Color::White);
			m_window.draw(tx);
		}
	}

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

	//loop thorugh the entire minimap grid
	for (int y = 0; y < mapHeight; ++y)
	{
		for (int x = 0; x < mapWidth; ++x)
		{
			const auto& room = m_mapGenerator.getRoom(x, y); //get room data from the generator

			bool visited = m_visitedRooms[y][x]; //check if the player visited this room

			if (!visited)
			{
				cell.setFillColor(sf::Color(30, 30, 30, 180)); // fog
			}
			else
			{
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
			}

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

std::string zombieStateToString(Zombie::State state)
{
	switch (state)
	{
	case Zombie::State::Idle:     return "Idle";
	case Zombie::State::Patrol:   return "Patrol";
	case Zombie::State::Chase:    return "Chase";
	case Zombie::State::Attack:   return "Attack";
	case Zombie::State::Cooldown: return "Cooldown";
	default:                      return "Unknown";
	}
}
