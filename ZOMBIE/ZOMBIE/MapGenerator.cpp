#include "MapGenerator.h"
#include <cstdlib>
#include <ctime>
#include <queue>
#include <iostream>

MapGenerator::MapGenerator(int roomsX, int roomsY, int roomSize)
    : m_roomsX(roomsX), m_roomsY(roomsY), m_roomSize(roomSize)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    m_rooms.resize(m_roomsY, std::vector<Room>(m_roomsX));
    m_roomShape.setSize(sf::Vector2f((float)m_roomSize, (float)m_roomSize));

    if (!m_wallTexture.loadFromFile("ASSETS/IMAGES/wall.png"))
        std::cout << "Failed to load wall texture\n";
    if (!m_floorTexture.loadFromFile("ASSETS/IMAGES/floor.png"))
        std::cout << "Failed to load floor texture\n";


    m_wallTexture.setRepeated(true);
    m_floorTexture.setRepeated(true);


}

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


// Generate the layout of rooms
void MapGenerator::generate()
{
    // Reset all rooms
    for (int y = 0; y < m_roomsY; ++y) //loop through each row
        for (int x = 0; x < m_roomsX; ++x) //loop through each column
            m_rooms[y][x] = Room(); //reset all the roooms

    //Build guaranteed downward path (main shaft)
    int startX = std::rand() % m_roomsX;
    int startY = 0;   //alway star from top row
    int x = startX;
    int y = startY;

    m_rooms[y][x].active = true;  //starting room active


    //path that always reaches the bottom of the grid
    while (y < m_roomsY - 1) 
    {
        int move = std::rand() % 3; // 0=left, 1=right, 2=down
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

            // must have matching exits both ways
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

// generate a 10×10 grid for a single room
void MapGenerator::generateRoomLayout(Room& room)
{
    const int WALL = 1;
    const int FLOOR = 0;
    const int LOCKED = 2; // new: locked door tile
    int width = Room::width;
    int height = Room::height;

    //10x10 grid filled with floor
    room.tiles.resize(height, std::vector<int>(width, FLOOR));

    //loop through grid
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //boarder = always walls
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
                room.tiles[i][j] = WALL;
            else
                //20% chance of a interior wall
                room.tiles[i][j] = (rand() % 100 < 20) ? WALL : FLOOR;
        }
    }

    //middle tile for door carving
    int mid = width / 2;

    // If room has exit Up - mark only the edge tile locked and ensure inner tile is floor
    if (room.exitUp)
    {
        for (int dx = -1; dx <= 1; ++dx) //door 3 tiles wide
        {
            int j = mid + dx;
            if (j >= 0 && j < width)
            {
                room.tiles[0][j] = LOCKED;
                // ensure tile just inside the room is floor (so locked barrier is single-tile thick)
                if (height > 1)
                    room.tiles[1][j] = FLOOR;
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

