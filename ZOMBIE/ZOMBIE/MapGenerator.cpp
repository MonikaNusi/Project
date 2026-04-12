#include "MapGenerator.h"
#include <cstdlib>
#include <ctime>
#include <queue>
#include <iostream>

//sets up the grid dimensions and loads textures
MapGenerator::MapGenerator(int roomsX, int roomsY, int roomSize)
    : m_roomsX(roomsX), m_roomsY(roomsY), m_roomSize(roomSize)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    // Create the 2D grid of rooms 
    m_rooms.resize(m_roomsY, std::vector<Room>(m_roomsX));
    // Shape used for debug rendering of rooms on the minimap
    m_roomShape.setSize(sf::Vector2f((float)m_roomSize, (float)m_roomSize));

    if (!m_tilesetTexture.loadFromFile("ASSETS/IMAGES/tileset.png"))
        std::cout << "Failed to load tileset texture\n";
    
    m_tilesetTexture.setSmooth(false);

    
    if (!m_wallTexture.loadFromFile("ASSETS/IMAGES/wall.png"))
        std::cout << "Failed to load wall texture\n";
    if (!m_floorTexture.loadFromFile("ASSETS/IMAGES/floor.png"))
        std::cout << "Failed to load floor texture\n";

    m_wallTexture.setRepeated(true);
    m_floorTexture.setRepeated(true);
}

// Returns a const reference to the room at grid position
const MapGenerator::Room& MapGenerator::getRoom(int x, int y) const
{
    return m_rooms[y][x];
}

void MapGenerator::setTile(int roomX, int roomY, int tileX, int tileY, int value)
{
    if (roomX < 0 || roomY < 0 || roomX >= m_rooms[0].size() || roomY >= m_rooms.size())
        return;
    Room& r = m_rooms[roomY][roomX];
    if (tileX < 0 || tileY < 0 || tileX >= Room::width || tileY >= Room::height)
        return;
    r.tiles[tileY][tileX] = value;
}

//Check if a tile position is a wall
bool MapGenerator::isWall(const Room& room, int x, int y) const
{
    if (x < 0 || y < 0 || x >= Room::width || y >= Room::height)
        return true; // treat out of bounds as walls
    
    return room.tiles[y][x] == 1 || room.tiles[y][x] == 2; // wall or locked door
}

//Count how many neighboring tiles are walls
int MapGenerator::countNeighborWalls(const Room& room, int x, int y) const
{
    int count = 0;
    
    // Check 8 directions
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dy == 0) continue; // skip center
            
            if (isWall(room, x + dx, y + dy))
                count++;
        }
    }
    
    return count;
}

// Determine which tile texture to use based on neighbors
sf::IntRect MapGenerator::getTileTextureRect(const Room& room, int tileX, int tileY) const
{
    const int TILE_SIZE = 16; // Base tile size
    
    int value = room.tiles[tileY][tileX];
    
    // floor tiles - 3 variations
    if (value == 0)
    {
        int seed = tileX * 1000 + tileY;
        int variant = (seed * 2654435761) % 3;
        
        switch (variant)
        {
            case 0: return sf::IntRect(32, 63, 32, 32); 
            case 1: return sf::IntRect(64, 63, 32, 32);  
            case 2: return sf::IntRect(80, 80, 32, 32); 
            case 3: return sf::IntRect(64, 63, 32, 32); 
            default: return sf::IntRect(80, 63, 32, 32);
        }
    }
    
    //locked door tiles check which type based on position
    if (value == 2)
    {
        int midX = Room::width / 2;
        int midY = Room::height / 2;
        
        if (tileX == 0)
        {
            // Left side door
            return sf::IntRect(19, 170, 32, 32);
        }
        else if (tileX == Room::width - 1)
        {
            // Right side door
            return sf::IntRect(37, 170, 32, 32);
        }
        else if (tileY == 0 || tileY == Room::height - 1)
        {
            // Front/back door
            return sf::IntRect(84, 170, 32, 32);
        }
        
        // generic locked door
        return sf::IntRect(7, 81, 32, 27);
    }
    
    //wall tiles
    bool isTopEdge = (tileY == 0);
    bool isBottomEdge = (tileY == Room::height - 1);
    bool isLeftEdge = (tileX == 0);
    bool isRightEdge = (tileX == Room::width - 1);
    
    // corners
    if (isTopEdge && isLeftEdge)
    {
        return sf::IntRect(0, 0, 32, 32); // top-left corner
    }
    
    if (isTopEdge && isRightEdge)
    {
        return sf::IntRect(160, 0, 32, 32); // top-right corner
    }
    
    // top edge walls
    if (isTopEdge)
    {
        return sf::IntRect(31, 0, 32, 32); // top wall tile
    }
    
    //left side wall
    if (isLeftEdge)
    {
        return sf::IntRect(0, 8, 32, 32); // left wall tile
    }
    
    // right side wall
    if (isRightEdge)
    {
        return sf::IntRect(160, 6, 32, 32); // right wall tile
    }
    
    // bottom edge wall
    if (isBottomEdge)
    {
        return sf::IntRect(32, 128, 32, 32);
    }
    
    return sf::IntRect(288, 63, 32, 32);
}

// Generate the layout of rooms
void MapGenerator::generate()
{
    // Reset all rooms
    for (int y = 0; y < m_roomsY; ++y)
        for (int x = 0; x < m_roomsX; ++x)
            m_rooms[y][x] = Room();

    //Build guaranteed downward path (main shaft)
    int startX = std::rand() % m_roomsX;
    int startY = 0;
    int x = startX;
    int y = startY;

    m_rooms[y][x].active = true;

    while (y < m_roomsY - 1) 
    {
        int move = std::rand() % 3;
        if (move == 0 && x > 0)
            x--; 
        else if (move == 1 && x < m_roomsX - 1)
            x++;
        else
            y++;

        m_rooms[y][x].active = true; 
    }

    // Random side rooms
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx) 
        {
            if (!m_rooms[yy][xx].active && std::rand() % 4 == 0)
                m_rooms[yy][xx].active = true;
        }
    }

    //Assign exits between adjacent active rooms
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            Room& r = m_rooms[yy][xx];
            if (!r.active) continue;

            if (xx < m_roomsX - 1 && m_rooms[yy][xx + 1].active)
            {
                r.exitRight = true;
                m_rooms[yy][xx + 1].exitLeft = true;
            }
            if (yy < m_roomsY - 1 && m_rooms[yy + 1][xx].active)
            {
                r.exitDown = true;
                m_rooms[yy + 1][xx].exitUp = true;
            }
        }
    }

    // Mark start & boss rooms
    sf::Vector2i startPos(startX, startY);

    // BFS to find reachable rooms and their distances
    std::vector<std::vector<int>> dist(m_roomsY, std::vector<int>(m_roomsX, -1));
    std::queue<sf::Vector2i> q;
    q.push(startPos);
    dist[startPos.y][startPos.x] = 0;

    const sf::Vector2i dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    while (!q.empty())
    {
        auto cur = q.front(); q.pop();
        const Room& r = m_rooms[cur.y][cur.x];

        for (auto d : dirs)
        {
            int nx = cur.x + d.x, ny = cur.y + d.y;
            if (nx < 0 || ny < 0 || nx >= m_roomsX || ny >= m_roomsY)
                continue;
            if (!m_rooms[ny][nx].active || dist[ny][nx] != -1)
                continue;

            if (d.x == 1 && !(r.exitRight && m_rooms[ny][nx].exitLeft)) continue;
            if (d.x == -1 && !(r.exitLeft && m_rooms[ny][nx].exitRight)) continue;
            if (d.y == 1 && !(r.exitDown && m_rooms[ny][nx].exitUp)) continue;
            if (d.y == -1 && !(r.exitUp && m_rooms[ny][nx].exitDown)) continue;

            dist[ny][nx] = dist[cur.y][cur.x] + 1;
            q.push({ nx, ny });
        }
    }

    // find the farthest reachable cell
    sf::Vector2i bossPos = startPos;
    int maxDist = 0;
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            if (dist[yy][xx] > maxDist)
            {
                maxDist = dist[yy][xx];
                bossPos = { xx, yy };
            }
        }
    }

    //assign colors/types
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            Room& room = m_rooms[yy][xx];
            if (!room.active) continue;

            if (xx == startPos.x && yy == startPos.y)
            {
                room.type = Room::RoomType::Start;
                room.color = sf::Color::Green;
            }
            else if (xx == bossPos.x && yy == bossPos.y)
            {
                room.type = Room::RoomType::Boss;
                room.color = sf::Color::Red;
            }
            else
            {
                int r = std::rand() % 100;
                if (r < 60)
                {
                    room.type = Room::RoomType::Normal;
                    room.color = sf::Color(100, 100, 150);
                }
                else if (r < 80)
                {
                    room.type = Room::RoomType::Treasure;
                    room.color = sf::Color(200, 180, 60);
                }
                else
                {
                    room.type = Room::RoomType::Trap;
                    room.color = sf::Color(180, 60, 60);
                }
            }
        }
    }

    //Generate interior layouts
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            if (m_rooms[yy][xx].active)
                generateRoomLayout(m_rooms[yy][xx]);
        }
    }
}

void MapGenerator::generateRoomLayout(Room& room)
{
    const int WALL = 1;
    const int FLOOR = 0;
    const int LOCKED = 2;
    int width = Room::width;
    int height = Room::height;

    room.tiles.resize(height, std::vector<int>(width, FLOOR));

    // First pass: borders and random interior
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //boarder = always wallsS
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
                room.tiles[i][j] = WALL;
            else
                //20% chance of a interior wall
                room.tiles[i][j] = (rand() % 100 < 20) ? WALL : FLOOR;
        }
    }

    //middle tile for door carving
    int mid = width / 2;

    // Clear areas around doors to prevent blocking
    if (room.exitUp)
    {
        for (int dx = -1; dx <= 1; ++dx) //door 3 tiles wide
        {
            int j = mid + dx;
            if (j >= 0 && j < width)
            {
                room.tiles[0][j] = LOCKED;
                // Clear 2 tiles deep to ensure access
                if (height > 1)
                    room.tiles[1][j] = FLOOR;
                if (height > 2)
                    room.tiles[2][j] = FLOOR;
            }
        }
    }

    if (room.exitDown)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int j = mid + dx;
            if (j >= 0 && j < width)
            {
                room.tiles[height - 1][j] = LOCKED;
                if (height > 1)
                    room.tiles[height - 2][j] = FLOOR;
                if (height > 2)
                    room.tiles[height - 3][j] = FLOOR;
            }
        }
    }

    if (room.exitLeft)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int i = mid + dy;
            if (i >= 0 && i < height)
            {
                room.tiles[i][0] = LOCKED;
                if (width > 1)
                    room.tiles[i][1] = FLOOR;
                if (width > 2)
                    room.tiles[i][2] = FLOOR;
            }
        }
    }

    if (room.exitRight)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int i = mid + dy;
            if (i >= 0 && i < height)
            {
                room.tiles[i][width - 1] = LOCKED;
                if (width > 1)
                    room.tiles[i][width - 2] = FLOOR;
                if (width > 2)
                    room.tiles[i][width - 3] = FLOOR;
            }
        }
    }
}

bool MapGenerator::isPathValid(const sf::Vector2i& start, const sf::Vector2i& goal) const
{
    //if either the start room or the goal room is not active path is not possible
    if (!m_rooms[start.y][start.x].active || !m_rooms[goal.y][goal.x].active)
        return false;
    //create a visited grid initialized to false
    std::vector<std::vector<bool>> visited(m_roomsY, std::vector<bool>(m_roomsX, false));
    std::queue<sf::Vector2i> q; //queve for BFS (rooms to check)
    q.push(start); // start BFS from the starting room
    visited[start.y][start.x] = true; //mark start as visited
    //directions we can move in the grid
    const sf::Vector2i dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    //standard BFS loop
    while (!q.empty())
    {
        auto cur = q.front();  //take next room from the queve
        q.pop();
        //if we reached the goal room, path is valid
        if (cur == goal) return true;

        const Room& r = m_rooms[cur.y][cur.x];
        //try moving in each of the 4 directions
        for (auto d : dirs)
        {
            int nx = cur.x + d.x, ny = cur.y + d.y;
            //if the neibhor is outside the grid skip it
            if (nx < 0 || ny < 0 || nx >= m_roomsX || ny >= m_roomsY)
                continue;
            //skip if room is inactive or has alrady been visited
            if (!m_rooms[ny][nx].active || visited[ny][nx])
                continue;

            // check corridor connection
            if (d.x == 1 && r.exitRight) { visited[ny][nx] = true; q.push({ nx, ny }); }
            if (d.x == -1 && r.exitLeft) { visited[ny][nx] = true; q.push({ nx, ny }); }
            if (d.y == 1 && r.exitDown) { visited[ny][nx] = true; q.push({ nx, ny }); }
            if (d.y == -1 && r.exitUp) { visited[ny][nx] = true; q.push({ nx, ny }); }
        }
    }
    return false;
}

//draw rooms
void MapGenerator::render(sf::RenderWindow& window)
{
    for (int y = 0; y < m_roomsY; ++y)
    {
        for (int x = 0; x < m_roomsX; ++x)
        {
            Room& room = m_rooms[y][x];
            float roomX = x * (m_roomSize + m_gap);
            float roomY = y * (m_roomSize + m_gap);

            // draw the room block
            if (room.active)
                m_roomShape.setFillColor(room.color);
            else
                m_roomShape.setFillColor(sf::Color(30, 30, 30));

            m_roomShape.setPosition(roomX, roomY);
            window.draw(m_roomShape);

            // draw connecting corridors that fill the gap exactly
            if (room.active)
            {
                if (room.exitRight)
                {
                    sf::RectangleShape cor(sf::Vector2f((float)m_gap, m_roomSize / 3.f));
                    cor.setPosition(roomX + m_roomSize, roomY + m_roomSize / 3.f);
                    cor.setFillColor(sf::Color(110, 110, 110));
                    window.draw(cor);
                }

                if (room.exitDown)
                {
                    sf::RectangleShape cor(sf::Vector2f(m_roomSize / 3.f, (float)m_gap));
                    cor.setPosition(roomX + m_roomSize / 3.f, roomY + m_roomSize);
                    cor.setFillColor(sf::Color(110, 110, 110));
                    window.draw(cor);
                }
            }
        }
    }
}

