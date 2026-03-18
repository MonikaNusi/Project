/// <summary>
/// @author Monika Nusi
/// @date 
/// </summary>

#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

static void drawLightOverlay(sf::RenderWindow &window, const sf::Vector2f &playerCenter, 
    const std::vector<sf::Vector2f>& torchPositions, int windowW, int windowH,
    int ambient, float playerRadius, float torchRadius)
{
    static sf::Texture lightTex;
    static bool generated = false;
    static const int texRadius = 256;

    if (!generated)
    {
        generated = true;
        const int texSize = texRadius * 2;
        sf::Image img;
        img.create(texSize, texSize, sf::Color::Black);

        for (int y = 0; y < texSize; ++y)
        {
            for (int x = 0; x < texSize; ++x)
            {
                float dx = (x + 0.5f - texRadius) / static_cast<float>(texRadius);
                float dy = (y + 0.5f - texRadius) / static_cast<float>(texRadius);
                float d = std::sqrt(dx * dx + dy * dy);

                float t = std::max(0.f, 1.0f - d);
                float brightness = t * t;

                sf::Uint8 v = static_cast<sf::Uint8>(255.f * brightness);
                img.setPixel(x, y, sf::Color(v, v, v, 255));
            }
        }

        lightTex.loadFromImage(img);
        lightTex.setSmooth(true);
    }

    static sf::RenderTexture lightMap;
    static bool lightMapCreated = false;
    static int cachedW = 0, cachedH = 0;

    if (!lightMapCreated || cachedW != windowW || cachedH != windowH)
    {
        lightMap.create(windowW, windowH);
        lightMapCreated = true;
        cachedW = windowW;
        cachedH = windowH;
    }

    const sf::Uint8 amb = static_cast<sf::Uint8>(ambient);
    lightMap.clear(sf::Color(amb, amb, amb, 255));

    lightMap.setView(window.getView());

    {
        sf::Sprite light(lightTex);
        light.setOrigin(static_cast<float>(texRadius), static_cast<float>(texRadius));
        light.setPosition(playerCenter);

        float scale = playerRadius / static_cast<float>(texRadius);
        light.setScale(scale, scale);
        light.setColor(sf::Color(255, 120, 20, 255));

        lightMap.draw(light, sf::BlendAdd);
    }

    // Torch lights (smaller orange glow)
    for (const auto& torchPos : torchPositions)
    {
        sf::Sprite light(lightTex);
        light.setOrigin(static_cast<float>(texRadius), static_cast<float>(texRadius));
        light.setPosition(torchPos);

        //const float torchRadius = 140.f;
        float scale = torchRadius / static_cast<float>(texRadius);
        light.setScale(scale, scale);
        light.setColor(sf::Color(200, 100, 15, 255));

        lightMap.draw(light, sf::BlendAdd);
    }

    lightMap.display();

    sf::Sprite overlay(lightMap.getTexture());

    window.setView(window.getDefaultView());
    window.draw(overlay, sf::BlendMultiply);

    window.setView(window.getView());
}

std::string zombieStateToString(Zombie::State state);


Game::Game(sf::RenderWindow& window, const DifficultySettings& difficulty) :
	m_window{ window },
	m_mapGenerator(8, 6, 100),
	m_difficulty{ difficulty }
{
	m_player.applyDifficulty(m_difficulty);

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
	

	if (!m_debugFont.loadFromFile("ASSETS/FONTS/zombie.ttf"))
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

	if (!m_keyTexture.loadFromFile("ASSETS/IMAGES/key.png"))
	{
		std::cout << "Failed to load key spritesheet\n";
	}

	if (!m_spikeTrapTexture.loadFromFile("ASSETS/IMAGES/trap.png"))
	{
		std::cout << "Failed to load spike trap texture\n";
	}
	m_spikeTrapTexture.setSmooth(false);


	if (!m_healthChestClosedTexture.loadFromFile("ASSETS/IMAGES/chestclosed.png"))
	{
		std::cout << "Failed to load health chest closed texture\n";
	}
	m_healthChestClosedTexture.setSmooth(false);

	if (!m_healthChestOpenTexture.loadFromFile("ASSETS/IMAGES/chestopen.png"))
	{
		std::cout << "Failed to load health chest open texture\n";
	}
	m_healthChestOpenTexture.setSmooth(false);

	if (!m_ammoChestClosedTexture.loadFromFile("ASSETS/IMAGES/chestclosed.png"))
	{
		std::cout << "Failed to load ammo chest closed texture\n";
	}
	m_ammoChestClosedTexture.setSmooth(false);

	if (!m_ammoChestOpenTexture.loadFromFile("ASSETS/IMAGES/chestopen.png"))
	{
		std::cout << "Failed to load ammo chest open texture\n";
	}
	m_ammoChestOpenTexture.setSmooth(false);

	if (!m_healthPotionTexture.loadFromFile("ASSETS/IMAGES/health.png"))
	{
		std::cout << "Failed to load health potion texture\n";
	}
	m_healthPotionTexture.setSmooth(false);

	if (!m_ammoCrateTexture.loadFromFile("ASSETS/IMAGES/ammo.png"))
	{
		std::cout << "Failed to load ammo crate texture\n";
	}
	m_ammoCrateTexture.setSmooth(false);

	if (!m_bone1Texture.loadFromFile("ASSETS/IMAGES/bones1.png"))
	{
		std::cout << "Failed to load bone1 texture\n";
	}
	m_bone1Texture.setSmooth(false);

	if (!m_bone2Texture.loadFromFile("ASSETS/IMAGES/bones2.png"))
	{
		std::cout << "Failed to load bone2 texture\n";
	}
	m_bone2Texture.setSmooth(false);

	// Load boss music
	if (!m_bossMusic.openFromFile("ASSETS/SOUNDS/boss.wav"))
	{
		std::cout << "Failed to load boss music\n";
	}
	else
	{
		m_bossMusic.setLoop(true);
		m_bossMusic.setVolume(50.f);
		std::cout << "Successfully loaded boss music\n";
	}

	Bullet::loadTexture();

	if (!m_torchTexture.loadFromFile("ASSETS/IMAGES/torch.png"))
	{
		std::cout << "Failed to load torch texture\n";
	}
	m_torchTexture.setSmooth(false);
}

Game::~Game()
{
	// Stop boss music when game ends
	if (m_bossMusic.getStatus() == sf::Music::Playing)
	{
		m_bossMusic.stop();
	}
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
				// Get mouse position
				sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
				sf::Vector2f mouseWorld = m_window.mapPixelToCoords(mousePixel, m_cameraView);
				
				// Calculate direction from player to mouse
				sf::Vector2f playerPos = m_player.getPosition();
				sf::Vector2f playerSize = m_player.getSize();
				sf::Vector2f playerCenter = playerPos + playerSize / 2.f;
				sf::Vector2f aimDir = mouseWorld - playerCenter;
				
				// Determine gun direction based on mouse angle
				Gun::Direction gunDir;
				if (std::abs(aimDir.x) > std::abs(aimDir.y))
				{
					// Mouse is more horizontal - use left or right
					gunDir = (aimDir.x > 0) ? Gun::Direction::Right : Gun::Direction::Left;
				}
				else
				{
					// Mouse is more vertical - use up or down
					gunDir = (aimDir.y > 0) ? Gun::Direction::Down : Gun::Direction::Up;
				}
				
				// Temporarily set gun direction for shooting
				m_player.setGunDirectionForShooting(gunDir);
				
				// Get bullet spawn position
				sf::Vector2f bulletSpawn = m_player.getGun().getBarrelPosition();

				m_player.shoot();
				m_bullets.emplace_back(bulletSpawn, aimDir);
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

void Game::updateBossMusic()
{
	const auto& currentRoomData = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	bool inBossRoom = (currentRoomData.type == MapGenerator::Room::RoomType::Boss);

	// Start music when entering boss room
	if (inBossRoom && !m_bossMusixPlaying && !m_bossDefeated)
	{
		m_bossMusic.play();
		m_bossMusixPlaying = true;
		std::cout << "Boss music started\n";
	}
	// Stop music when leaving boss room or boss is defeated
	else if ((!inBossRoom || m_bossDefeated) && m_bossMusixPlaying)
	{
		m_bossMusic.stop();
		m_bossMusixPlaying = false;
		std::cout << "Boss music stopped\n";
	}
}

void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();
	}

	updateBossMusic();

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
		// Check if death animation is complete
		if (m_zombies[i].isDeathAnimationComplete())
		{
			if (m_zombies[i].hasKey())
			{
				// spawn animated key at zombie position
				Key newKey(m_zombies[i].getPosition());
				newKey.sprite.setTexture(m_keyTexture);
				newKey.sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(m_keyFrameWidth), 47));
				m_keys.push_back(newKey);
				std::cout << "Spawned key from dead zombie\n";
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

		if (b.isAlive() && isCollidingWithWall(b.getBounds()))
		{
			b.kill();
		}
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

	// Bullet collision with boss zombie
	if (m_bossZombie)
	{
		for (auto& b : m_bullets)
		{
			if (!b.isAlive()) continue;

			if (b.getBounds().intersects(m_bossZombie->getHitbox()))
			{
				m_bossZombie->takeDamage(25);
				b.kill();
				std::cout << "Hit boss!\n";
			}
		}
	}

	// Update spike traps
	for (auto& trap : m_spikeTraps)
	{
		trap.update(t_deltaTime);

		// Check collision with player
		if (trap.getBounds().intersects(playerBox))
		{
			if (!trap.isActive())
			{
				trap.trigger();
				m_player.takeDamage(trap.getDamage());
				std::cout << "Player hit spike trap! Health: " << m_player.getHealth() << "\n";
			}
		}
	}

	// Update pickups
	for (auto& pickup : m_pickups)
	{
		pickup.update(t_deltaTime);

		// health and ammo chests
		if (!pickup.isOpened() && !pickup.isPicked())
		{
			if (pickup.isNearby(playerBox, 60.f))
			{
				// Player is near chest check for E key
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
				{
					pickup.open();
					
					// Create dropped item instead
					DroppedItem::Type itemType = (pickup.getType() == Pickup::Type::Health) 
						? DroppedItem::Type::Health 
						: DroppedItem::Type::Ammo;
					
					// Calculate spawn position
					sf::Vector2f chestPos = pickup.getPosition();
					sf::Vector2f itemSpawnPos = chestPos;
					itemSpawnPos.y -= 100.f;
					
					DroppedItem item(itemType, pickup.getValue(), itemSpawnPos);
					
					// Set appropriate texture
					if (itemType == DroppedItem::Type::Health)
					{
						item.sprite.setTexture(m_healthPotionTexture);
					}
					else
					{
						item.sprite.setTexture(m_ammoCrateTexture);
					}
					
					// Setup sprite
					item.sprite.setPosition(itemSpawnPos);
					item.sprite.setOrigin(
						item.sprite.getTexture()->getSize().x / 2.f,
						item.sprite.getTexture()->getSize().y / 2.f
					);
					item.sprite.setScale(2.f, 2.f);
					
					m_droppedItems.push_back(item);
					
					std::cout << "Opened chest! Item dropped at (" << itemSpawnPos.x << ", " << itemSpawnPos.y << ")\n";
				}
			}
		}
	}

	// Animate torches
	for (auto& torch : m_torches)
	{
		torch.frameTimer += t_deltaTime.asSeconds();
		if (torch.frameTimer >= m_torchFrameTime)
		{
			torch.frameTimer = 0.f;
			torch.currentFrame = (torch.currentFrame + 1) % m_torchFrameCount;
			torch.sprite.setTextureRect(sf::IntRect(
				static_cast<int>(torch.currentFrame * m_torchFrameWidth), 0,
				static_cast<int>(m_torchFrameWidth),
				static_cast<int>(m_torchFrameHeight)));
		}
	}


	// Update and handle dropped items
	for (auto& item : m_droppedItems)
	{
		if (item.picked) continue;
		
		// Bobbing animation
		item.bobTimer += t_deltaTime.asSeconds() * 2.f;
		float offset = std::sin(item.bobTimer) * 5.f;
		sf::Vector2f pos = item.sprite.getPosition();
		pos.y = item.baseY + offset;
		item.sprite.setPosition(pos);
		
		// Check pickup
		sf::FloatRect itemRect = item.sprite.getGlobalBounds();
		if (itemRect.intersects(playerBox))
		{
			item.picked = true;
			
			if (item.type == DroppedItem::Type::Health)
			{
				m_player.heal(item.value);
				std::cout << "Picked up health potion! +" << item.value << " HP\n";
			}
			else
			{
				m_player.addAmmo(item.value);
				std::cout << "Picked up ammo! +" << item.value << "\n";
			}
		}
	}

	// Remove picked items
	m_droppedItems.erase(
		std::remove_if(m_droppedItems.begin(), m_droppedItems.end(),
			[](const DroppedItem& i) { return i.picked; }),
		m_droppedItems.end()
	);


	//Animate keys
	for (auto& key : m_keys)
	{
		if (key.picked) continue;
		
		key.frameTimer += t_deltaTime.asSeconds();
		
		// Switch to next frame when timer exceeds frame time
		if (key.frameTimer >= m_keyFrameTime)
		{
			key.frameTimer = 0.f;
			key.currentFrame = (key.currentFrame + 1) % m_keyFrameCount;
			
			// Update texture rect for current frame
			sf::IntRect frameRect(
				static_cast<int>(key.currentFrame * m_keyFrameWidth), 
				0, 
				static_cast<int>(m_keyFrameWidth), 
				47  // height of spritesheet
			);
			key.sprite.setTextureRect(frameRect);
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

					//detection radius
					if (dist < 120.f)
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
				std::cout << "Started unlocking door at (" << doorRoomX << "," << doorRoomY << ")\n";
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

			loadRoomState();

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

		//player is at right edge and this room has a right exit and the door tile is not locked
		if (center.x > windowW - margin && current.exitRight)
		{
			// Check the border tile for locked status
			if (current.tiles[midY][current.width - 1] == 0)
				newRoom.x++;
		}
		//player is at left edge and this room has a left exit and not locked
		else if (center.x < margin && current.exitLeft)
		{
			if (current.tiles[midY][0] == 0)
				newRoom.x--;
		}
		//player is at bottom edge and this room has a down exit and not locked
		else if (center.y > windowH - margin && current.exitDown)
		{
			if (current.tiles[current.height - 1][midX] == 0)
				newRoom.y++;
		}
		//player is at top edge and this room has an up exit and not locked
		else if (center.y < margin && current.exitUp)
		{
			if (current.tiles[0][midX] == 0)
				newRoom.y--;
		}

		//if newRoom changed player endered the door
		if (newRoom != m_currentRoom)
		{
			// Save current room state before leaving
			saveCurrentRoomState();
			
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

	// Update boss zombie
	if (m_bossZombie)
	{
		sf::Vector2f oldBossPos = m_bossZombie->getPosition();
		
		const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
		float tileW = (float)m_window.getSize().x / room.width;
		float tileH = (float)m_window.getSize().y / room.height;
		
		m_bossZombie->update(t_deltaTime, playerCenter, room.tiles, tileW, tileH);
		
		// Boss collision with walls
		if (isCollidingWithWall(m_bossZombie->getHitbox()))
		{
			sf::Vector2f newPos = m_bossZombie->getPosition();
			
			// Try moving only horizontally
			m_bossZombie->setPosition(newPos.x, oldBossPos.y);
			if (isCollidingWithWall(m_bossZombie->getHitbox()))
			{
				// Try moving only vertically
				m_bossZombie->setPosition(oldBossPos.x, newPos.y);
				if (isCollidingWithWall(m_bossZombie->getHitbox()))
				{
					m_bossZombie->setPosition(oldBossPos.x, oldBossPos.y);
				}
			}
		}
		
		// Boss attack player
		int dmg = m_bossZombie->tryDealDamage(m_debugPlayerBox);
		if (dmg > 0)
		{
			m_player.takeDamage(dmg);
			std::cout << "Boss dealt " << dmg << " damage! Player Health: " << m_player.getHealth() << "\n";
		}
		
		// Check if boss is dead
		if (m_bossZombie->isDeathAnimationComplete())
		{
			std::cout << "BOSS DEFEATED!\n";
			m_bossDefeated = true;
			m_bossZombie.reset();
		}
	}

	if (m_bossZombie && m_bossZombie->shouldSpawnMinions())
	{
		m_bossZombie->clearMinionSpawnFlag();
		
		// Spawn minions around the boss
		sf::Vector2f bossPos = m_bossZombie->getPosition();
		float spawnRadius = 150.f;
		int minionCount = 3;
		
		for (int i = 0; i < minionCount; ++i)
		{
			float angle = (360.f / minionCount) * i;
			float radians = angle * 3.14159f / 180.f;
			
			sf::Vector2f spawnOffset(
				std::cos(radians) * spawnRadius,
				std::sin(radians) * spawnRadius
			);
			
			Minion minion(bossPos + spawnOffset);
			minion.applyDifficulty(m_difficulty.minionHealth, m_difficulty.minionDamage);
			m_minions.push_back(minion);
		}
		
		std::cout << "Spawned " << minionCount << " minions!\n";
	}

	// Update minions
	for (auto& minion : m_minions)
	{
		sf::Vector2f oldMinionPos = minion.getPosition();
		
		const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
		float tileW = (float)m_window.getSize().x / room.width;
		float tileH = (float)m_window.getSize().y / room.height;
		
		minion.update(t_deltaTime, playerCenter, room.tiles, tileW, tileH);
		
		// Minion collision with walls
		if (isCollidingWithWall(minion.getHitbox()))
		{
			sf::Vector2f newPos = minion.getPosition();
			
			// Try X-axis only
			minion.setPosition(newPos.x, oldMinionPos.y);
			if (isCollidingWithWall(minion.getHitbox()))
			{
				// Try Y-axis only
				minion.setPosition(oldMinionPos.x, newPos.y);
				if (isCollidingWithWall(minion.getHitbox()))
				{
					// Both axes collide, revert completely
					minion.setPosition(oldMinionPos.x, oldMinionPos.y);
				}
			}
		}
		
		// Minion attack player
		int dmg = minion.tryDealDamage(m_debugPlayerBox);
		if (dmg > 0)
		{
			m_player.takeDamage(dmg);
			std::cout << "Minion dealt " << dmg << " damage! Player Health: " << m_player.getHealth() << "\n";
		}
	}

	// Bullet collision with minions
	for (auto& b : m_bullets)
	{
		if (!b.isAlive()) continue;

		for (auto& minion : m_minions)
		{
			if (minion.isDead()) continue;

			if (b.getBounds().intersects(minion.getHitbox()))
			{
				minion.takeDamage(25);
				b.kill();
				std::cout << "Hit minion!\n";
			}
		}
	}

	// Remove dead minions
	m_minions.erase(
		std::remove_if(m_minions.begin(), m_minions.end(),
			[](const Minion& m) { return m.isDeathAnimationComplete(); }),
		m_minions.end()
	);


	if (m_player.getHealth() <= 0)
	{
		std::cout << "GAME OVER - Player Health: " << m_player.getHealth() << "\n";
		m_exitGame = true;
	}
}

void Game::spawnZombiesForRoom()
{
	// Check if zombies spawned for this room
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	if (m_roomZombies.find(roomKey) != m_roomZombies.end())
	{
		return;
	}


	m_zombies.clear();
	m_minions.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);

	int zombieCount = 0;
	int minionCount = 0;


	switch (room.type)
	{
	case MapGenerator::Room::RoomType::Normal:
		zombieCount = m_difficulty.zombieCountNormal;
		minionCount = m_difficulty.minionCountNormal + (std::rand() % 2);
		break;
	case MapGenerator::Room::RoomType::Trap:
		zombieCount = m_difficulty.zombieCountTrap;
		minionCount = m_difficulty.minionCountTrap + (std::rand() % 3);
		break;
	case MapGenerator::Room::RoomType::Treasure:
		zombieCount = m_difficulty.zombieCountTreasure;
		minionCount = m_difficulty.minionCountTreasure + (std::rand() % 2);
		break;
	case MapGenerator::Room::RoomType::Boss:
		zombieCount = 0;
		minionCount = 0;

		if (!m_bossDefeated)
		{
			sf::Vector2f bossSpawn = findSafeSpawn(room);
			m_bossZombie = std::make_unique<BossZombie>(bossSpawn);
			m_bossZombie->applyDifficulty(m_difficulty.bossHealth);
			std::cout << "Boss zombie spawned!\n";
		}
		break;
	case MapGenerator::Room::RoomType::Start:
		zombieCount = m_difficulty.zombieCountStart;
		minionCount = m_difficulty.minionCountStart;
		break;
	default:
		zombieCount = 1;
		minionCount = 1;
		break;
	}

	std::vector<sf::Vector2f> usedPositions;
	const float minSpawnDistance = 80.f;

	// Spawn zombies
	for (int i = 0; i < zombieCount; ++i)
	{
		sf::Vector2f spawn = findUniqueSpawn(room, usedPositions, minSpawnDistance);
		usedPositions.push_back(spawn);
		
		Zombie z(m_zombieTexture);
		z.reset(spawn);
		z.applyDifficulty(m_difficulty.zombieHealth, m_difficulty.zombieDamage);
		m_zombies.push_back(z);
	}

	for (int i = 0; i < minionCount; ++i)
	{
		sf::Vector2f spawn = findUniqueSpawn(room, usedPositions, minSpawnDistance);
		usedPositions.push_back(spawn);
		
		Minion minion(spawn);
		minion.applyDifficulty(m_difficulty.minionHealth, m_difficulty.minionDamage);
		m_minions.push_back(minion);
	}

	if (!m_zombies.empty())
	{
		int idx = std::rand() % static_cast<int>(m_zombies.size());
		m_zombies[idx].setHasKey(true);
		std::cout << "Assigned key to zombie " << idx << "\n";
	}

	// Store the initial state for this room
	m_roomZombies[roomKey] = m_zombies;
	m_roomMinions[roomKey] = m_minions;
	
	std::cout << "Spawned " << zombieCount << " zombies and " << minionCount 
		<< " minions in room (" << m_currentRoom.x << "," << m_currentRoom.y << ")\n";
}

void Game::saveCurrentRoomState()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	// Save current zombies
	m_roomZombies[roomKey] = m_zombies;
	
	// Save current minions
	m_roomMinions[roomKey] = m_minions;
	
	// Save current traps
	m_roomTraps[roomKey] = m_spikeTraps;
	
	// Save current pickups
	m_roomPickups[roomKey] = m_pickups;
	
	// Save dropped items
	m_roomDroppedItems[roomKey] = m_droppedItems;
	
	// Save decorations
	m_roomDecorations[roomKey] = m_decorations;
	
	// Save current keys
	m_roomKeys[roomKey] = m_keys;

	m_roomTorches[roomKey] = m_torches;
	
	std::cout << "Saved room (" << m_currentRoom.x << "," << m_currentRoom.y 
		<< ") with " << m_zombies.size() << " zombies and " << m_minions.size() << " minions\n";
}

void Game::loadRoomState()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	// Check if this room has been visited before
	bool roomExists = (m_roomZombies.find(roomKey) != m_roomZombies.end());
	
	if (roomExists)
	{
		// Room already visited - load saved state
		m_zombies = m_roomZombies[roomKey];
		m_minions = m_roomMinions[roomKey];
		
		std::cout << "Loaded room (" << m_currentRoom.x << "," << m_currentRoom.y 
			<< ") with " << m_zombies.size() << " zombies and " << m_minions.size() << " minions\n";
	}
	else
	{

		spawnZombiesForRoom();
		std::cout << "First visit - spawned " << m_zombies.size() << " zombies and " 
			<< m_minions.size() << " minions\n";
	}
	
	// Load traps for this room
	if (m_roomTraps.find(roomKey) != m_roomTraps.end())
	{
		m_spikeTraps = m_roomTraps[roomKey];
	}
	else
	{
		spawnTrapsForRoom();
	}
	
	// Load pickups for this room
	if (m_roomPickups.find(roomKey) != m_roomPickups.end())
	{
		m_pickups = m_roomPickups[roomKey];
	}
	else
	{
		spawnPickupsForRoom();
	}
	
	// Load decorations for this room
	if (m_roomDecorations.find(roomKey) != m_roomDecorations.end())
	{
		m_decorations = m_roomDecorations[roomKey];
	}
	else
	{
		spawnDecorationsForRoom();
	}
	
	// Load dropped items for this room
	if (m_roomDroppedItems.find(roomKey) != m_roomDroppedItems.end())
	{
		m_droppedItems = m_roomDroppedItems[roomKey];
	}
	else
	{
		m_droppedItems.clear();
	}
	
	// Load keys if they exist for this room
	if (m_roomKeys.find(roomKey) != m_roomKeys.end())
	{
		m_keys = m_roomKeys[roomKey];
	}
	else
	{
		m_keys.clear();
	}

	if (m_roomTorches.find(roomKey) != m_roomTorches.end())
	{
		m_torches = m_roomTorches[roomKey];
	}
	else
	{
		spawnTorchesForRoom();
	}
}

void Game::spawnTrapsForRoom()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	// Check if traps already spawned for this room
	if (m_roomTraps.find(roomKey) != m_roomTraps.end())
	{
		return;
	}

	m_spikeTraps.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	int trapCount = 0;

	// Spawn more traps in Trap-type rooms
	switch (room.type)
	{
	case MapGenerator::Room::RoomType::Trap:
		trapCount = m_difficulty.trapCountTrap;
		break;
	case MapGenerator::Room::RoomType::Normal:
		trapCount = m_difficulty.trapCountNormal;
		break;
	case MapGenerator::Room::RoomType::Boss:
		trapCount = m_difficulty.trapCountBoss;
		break;
	default:
		trapCount = m_difficulty.trapCountDefault;
		break;
	}

	// Place traps on random floor tiles
	for (int i = 0; i < trapCount; ++i)
	{
		int attempts = 0;
		while (attempts < 20)  // Try 20 times to find a valid spot
		{
			int x = 2 + (std::rand() % (room.width - 4));
			int y = 2 + (std::rand() % (room.height - 4));

			// Only place on floor tiles
			if (room.tiles[y][x] == 0)
			{
				sf::Vector2f trapPos(
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				);

				SpikeTrap trap(trapPos, m_spikeTrapTexture);
				m_spikeTraps.push_back(trap);
				break;
			}
			attempts++;
		}
	}

	// Store traps for this room
	m_roomTraps[roomKey] = m_spikeTraps;

	std::cout << "Spawned " << m_spikeTraps.size() << " spike traps in room (" 
		<< m_currentRoom.x << "," << m_currentRoom.y << ")\n";
}

void Game::spawnPickupsForRoom()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	// Check if pickups already spawned for this room
	if (m_roomPickups.find(roomKey) != m_roomPickups.end())
	{
		return;
	}

	m_pickups.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	// Determine pickup counts based on room type
	int healthCount = 0;
	int ammoCount = 0;

	switch (room.type)
	{
	case MapGenerator::Room::RoomType::Treasure:
		healthCount = 3; 
		ammoCount = 3;   
		break;
	case MapGenerator::Room::RoomType::Boss:
		healthCount = 2;  
		ammoCount = 2;   
		break;
	case MapGenerator::Room::RoomType::Normal:
		healthCount = 2;  
		ammoCount = 2;   
		break;
	case MapGenerator::Room::RoomType::Trap:
		healthCount = 2; 
		ammoCount = 2;   
		break;
	case MapGenerator::Room::RoomType::Start:
		healthCount = 1;
		ammoCount = 1;
		break;
	default:
		healthCount = 1;
		ammoCount = 1;
		break;
	}

	int healthRange = m_difficulty.healthPickupMax - m_difficulty.healthPickupMin + 1;
	int ammoRange = m_difficulty.ammoPickupMax - m_difficulty.ammoPickupMin + 1;

	for (int i = 0; i < healthCount; ++i)
	{
		int attempts = 0;
		while (attempts < 30)
		{
			int x = 2 + (std::rand() % (room.width - 4));
			int y = 2 + (std::rand() % (room.height - 4));

			if (room.tiles[y][x] == 0)  // Floor tile
			{
				sf::Vector2f pickupPos(
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				);

				int healAmount = m_difficulty.healthPickupMin + (std::rand() % healthRange);
				Pickup pickup(pickupPos, Pickup::Type::Health, healAmount, 
					m_healthChestClosedTexture, m_healthChestOpenTexture);
				m_pickups.push_back(pickup);
				break;
			}
			attempts++;
		}
	}

	// Spawn ammo chests
	for (int i = 0; i < ammoCount; ++i)
	{
		int attempts = 0;
		while (attempts < 30)
		{
			int x = 2 + (std::rand() % (room.width - 4));
			int y = 2 + (std::rand() % (room.height - 4));

			if (room.tiles[y][x] == 0)  // Floor tile
			{
				sf::Vector2f pickupPos(
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				);

				int ammoAmount = m_difficulty.ammoPickupMin + (std::rand() % ammoRange);
				Pickup pickup(pickupPos, Pickup::Type::Ammo, ammoAmount, 
					m_ammoChestClosedTexture, m_ammoChestOpenTexture);
				m_pickups.push_back(pickup);
				break;
			}
			attempts++;
		}
	}

	// Store pickups for this room
	m_roomPickups[roomKey] = m_pickups;

	std::cout << "Spawned " << healthCount << " health and " << ammoCount 
		<< " ammo pickups in room (" << m_currentRoom.x << "," << m_currentRoom.y << ")\n";
}

void Game::spawnDecorationsForRoom()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);
	
	// Check if decorations already spawned for this room
	if (m_roomDecorations.find(roomKey) != m_roomDecorations.end())
	{
		return;
	}

	m_decorations.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	// Determine decoration count based on room type
	int boneCount = 0;

	switch (room.type)
	{
	case MapGenerator::Room::RoomType::Treasure:
		boneCount = 2;
		break;
	case MapGenerator::Room::RoomType::Boss:
		boneCount = 8;
		break;
	case MapGenerator::Room::RoomType::Normal:
		boneCount = 3 + (std::rand() % 3);
		break;
	case MapGenerator::Room::RoomType::Trap:
		boneCount = 5 + (std::rand() % 4);
		break;
	case MapGenerator::Room::RoomType::Start:
		boneCount = 2;
		break;
	default:
		boneCount = 2;
		break;
	}

	// Spawn random bones around the room
	for (int i = 0; i < boneCount; ++i)
	{
		int attempts = 0;
		while (attempts < 30)
		{
			int x = 1 + (std::rand() % (room.width - 2));
			int y = 1 + (std::rand() % (room.height - 2));

			if (room.tiles[y][x] == 0)  // Floor tile
			{
				sf::Vector2f decorationPos(
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				);

				Decoration decoration(decorationPos);
				
				// Randomly choose between bone1 and bone2
				bool useBone1 = (std::rand() % 2 == 0);
				if (useBone1)
				{
					decoration.sprite.setTexture(m_bone1Texture);
				}
				else
				{
					decoration.sprite.setTexture(m_bone2Texture);
				}
				
				// Center origin
				decoration.sprite.setOrigin(
					decoration.sprite.getTexture()->getSize().x / 2.f,
					decoration.sprite.getTexture()->getSize().y / 2.f
				);
				
				decoration.sprite.setPosition(decorationPos);
				
				float scale = 1.0f + static_cast<float>(std::rand() % 70) / 100.f;
				decoration.sprite.setScale(scale, scale);
				
				// Random rotation
				float rotation = static_cast<float>(std::rand() % 360);
				decoration.sprite.setRotation(rotation);
				
				m_decorations.push_back(decoration);
				break;
			}
			attempts++;
		}
	}



	// Store decorations for this room
	m_roomDecorations[roomKey] = m_decorations;
	

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

	if (room.tiles[cy][cx] == 0) //floor
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
	// Coming from right - spawn at right doorww
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
	    

	    sf::Sprite tileSprite;
	    tileSprite.setTexture(m_mapGenerator.getTilesetTexture());
	    
	    for (int i = 0; i < room.height; ++i)
	    {
	        for (int j = 0; j < room.width; ++j)
	        {
	            // Get the appropriate texture rect based on tile neighbors
	            sf::IntRect texRect = m_mapGenerator.getTileTextureRect(room, j, i);
	            
	            tileSprite.setTextureRect(texRect);
	            
	            // Position the sprite
	            tileSprite.setPosition(offset.x + j * tileW, offset.y + i * tileH);
	            
	            // Scale sprite to fit tile size
	            float scaleX = tileW / texRect.width;
	            float scaleY = tileH / texRect.height;
	            tileSprite.setScale(scaleX, scaleY);
	            
            
	            if (room.tiles[i][j] == 2) // locked door
	            {
	                //tileSprite.setColor(sf::Color(200, 160, 255));
	            }
	            else
	            {
	                tileSprite.setColor(sf::Color::White); // normal rendering
	            }
	            
	            m_window.draw(tileSprite);
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

	// Draw spike traps
	for (auto& trap : m_spikeTraps)
	{
		trap.render(m_window);
	}

	// Draw decorations
	for (const auto& decoration : m_decorations)
	{
		m_window.draw(decoration.sprite);
	}

	// Draw pickups
	for (auto& pickup : m_pickups)
	{
		pickup.render(m_window);
	}

	//m_mapGenerator.render(m_window);
	renderPickupPrompts();

	// Draw player
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

		// Draw zombie hitbox
		sf::RectangleShape hb;
		auto zb = z.getHitbox();
		hb.setPosition(zb.left, zb.top);
		hb.setSize({ zb.width, zb.height });
		hb.setFillColor(sf::Color(0, 255, 0, 120));
		m_window.draw(hb);
	}

	// Draw boss zombie
	if (m_bossZombie)
	{
		m_bossZombie->render(m_window);

		// Debug hitbox for boss
		sf::RectangleShape hb;
		auto bossHitbox = m_bossZombie->getHitbox();
		hb.setPosition(bossHitbox.left, bossHitbox.top);
		hb.setSize({ bossHitbox.width, bossHitbox.height });
		hb.setFillColor(sf::Color(255, 0, 255, 120));
		m_window.draw(hb);
	}

	// Draw minions
	for (auto& minion : m_minions)
	{
		minion.render(m_window);

		// Debug hitbox for minion
		sf::RectangleShape hb;
		auto minionHitbox = minion.getHitbox();
		hb.setPosition(minionHitbox.left, minionHitbox.top);
		hb.setSize({ minionHitbox.width, minionHitbox.height });
		hb.setFillColor(sf::Color(255, 255, 0, 120));
		m_window.draw(hb);
	}

	// draw dropped keys with animation
	for (const auto& key : m_keys)
	{
		if (!key.picked)
		{
			m_window.draw(key.sprite);
		}
	}

	// Draw dropped items from chests
	for (const auto& item : m_droppedItems)
	{
		if (!item.picked)
		{
			m_window.draw(item.sprite);
		}
	}

	// Draw bullets
	for (const auto& b : m_bullets)
	{
		b.render(m_window);
	}

	for (const auto& torch : m_torches)
	{
		m_window.draw(torch.sprite);
	}

	sf::Vector2f playerPos = m_player.getPosition();
	sf::Vector2f playerSize = m_player.getSize();
	sf::Vector2f playerCenter = playerPos + playerSize / 2.f;

	// Collect torch positions for light overlay
	std::vector<sf::Vector2f> torchPositions;
	for (const auto& torch : m_torches)
	{
		torchPositions.push_back(torch.pos);
	}

	drawLightOverlay(m_window, playerCenter, torchPositions, windowW, windowH, m_difficulty.ambientLight, m_difficulty.playerLightRadius, m_difficulty.torchLightRadius);

	m_window.setView(m_cameraView);

	m_window.setView(m_window.getDefaultView());

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

// Draw "Press E" prompt for nearby chests
void Game::renderPickupPrompts()
{
    for (const auto& pickup : m_pickups)
    {
        if (!pickup.isOpened() && !pickup.isPicked())
        {
            if (pickup.isNearby(m_debugPlayerBox, 60.f))
            {
                std::string promptText = (pickup.getType() == Pickup::Type::Health)
                    ? "Press E to open (Health)"
					: "Press E to open (Ammo)" ;

                sf::Text prompt(promptText, m_uiFont, 16);
                prompt.setFillColor(sf::Color::White);
                prompt.setPosition(
                    pickup.getPosition().x - 70.f,
                    pickup.getPosition().y - 40.f
                );
                m_window.draw(prompt);
            }
        }
    }
}

void Game::spawnTorchesForRoom()
{
	auto roomKey = std::make_pair(m_currentRoom.x, m_currentRoom.y);

	if (m_roomTorches.find(roomKey) != m_roomTorches.end())
	{
		return;
	}

	m_torches.clear();

	const auto& room = m_mapGenerator.getRoom(m_currentRoom.x, m_currentRoom.y);
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	std::vector<sf::Vector2f> candidates;

	for (int y = 0; y < room.height - 1; ++y)
	{
		for (int x = 1; x < room.width - 1; ++x)
		{
			if (room.tiles[y][x] != 1) continue;

			bool floorBelow = (y + 1 < room.height && room.tiles[y + 1][x] == 0);

			if (floorBelow)
			{
				sf::Vector2f torchPos(
					x * tileW + tileW * 0.5f,
					y * tileH + tileH * 0.5f
				);
				candidates.push_back(torchPos);
			}
		}
	}

	int torchCount = std::min(static_cast<int>(candidates.size()), 4 + (std::rand() % 3));

	for (int i = static_cast<int>(candidates.size()) - 1; i > 0; --i)
	{
		int j = std::rand() % (i + 1);
		std::swap(candidates[i], candidates[j]);
	}

	for (int i = 0; i < torchCount && i < static_cast<int>(candidates.size()); ++i)
	{
		Torch torch(candidates[i]);
		torch.sprite.setTexture(m_torchTexture);
		torch.sprite.setTextureRect(sf::IntRect(0, 0,
			static_cast<int>(m_torchFrameWidth),
			static_cast<int>(m_torchFrameHeight)));
		torch.sprite.setOrigin(m_torchFrameWidth / 2.f, m_torchFrameHeight / 2.f);
		torch.sprite.setPosition(candidates[i]);
		torch.sprite.setScale(2.f, 2.f);

		torch.currentFrame = std::rand() % m_torchFrameCount;
		torch.frameTimer = static_cast<float>(std::rand() % 100) / 100.f * m_torchFrameTime;

		m_torches.push_back(torch);
	}

	m_roomTorches[roomKey] = m_torches;
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

sf::Vector2f Game::findUniqueSpawn(const MapGenerator::Room& room, 
	const std::vector<sf::Vector2f>& usedPositions, 
	float minDistance)
{
	const int windowW = m_window.getSize().x;
	const int windowH = m_window.getSize().y;

	float tileW = (float)windowW / room.width;
	float tileH = (float)windowH / room.height;

	// Try up to 50 times to find a unique spawn position
	for (int attempt = 0; attempt < 50; attempt++)
{
		// Generate random position on a floor tile
		int x =  2 + (std::rand() % (room.width - 4));
		int y = 2 + (std::rand() % (room.height - 4));

		if (room.tiles[y][x] != 0) continue;  // Not a floor tile

		sf::Vector2f candidate(
			x * tileW + tileW * 0.5f,
			y * tileH + tileH * 0.5f
		);

		// Check if this position is far enough from all used positions
		bool validPosition = true;
		for (const auto& usedPos : usedPositions)
		{
			float dx = candidate.x - usedPos.x;
			float dy = candidate.y - usedPos.y;
			float distSq = dx * dx + dy * dy;

			if (distSq < minDistance * minDistance)
			{
				validPosition = false;
				break;
			}
		}

		if (validPosition)
		{
			return candidate;
		}
	}

	return findSafeSpawn(room);
}
