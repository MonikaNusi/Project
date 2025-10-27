#include "MapGenerator.h"
#include <cstdlib>
#include <ctime>

MapGenerator::MapGenerator(int roomsX, int roomsY, int roomSize)
    : m_roomsX(roomsX), m_roomsY(roomsY), m_roomSize(roomSize)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    m_rooms.resize(m_roomsY, std::vector<Room>(m_roomsX));
    m_roomShape.setSize(sf::Vector2f((float)m_roomSize, (float)m_roomSize));
}

const MapGenerator::Room& MapGenerator::getRoom(int x, int y) const
{
    return m_rooms[y][x];
}


// Generate the layout of rooms

void MapGenerator::generate()
{
    // Reset all rooms
    for (int y = 0; y < m_roomsY; ++y)
        for (int x = 0; x < m_roomsX; ++x)
            m_rooms[y][x] = Room();

    int startX = std::rand() % m_roomsX;
    int startY = 0;
    int x = startX;
    int y = startY;
    bool movingRight = (std::rand() % 2 == 0);
    bool finished = false;

    // Build downward until bottom row reached
    while (!finished)
    {
        m_rooms[y][x].active = true;
        m_rooms[y][x].type = 1; // horizontal default

        int move = std::rand() % 5;
        if (move <= 1) // move left
        {
            if (x > 0)
                x--;
            else
            {
                y++;
                movingRight = true;
            }
        }
        else if (move <= 3) // move right
        {
            if (x < m_roomsX - 1)
                x++;
            else
            {
                y++;
                movingRight = false;
            }
        }
        else // move down
        {
            y++;
        }

        if (y >= m_roomsY)
        {
            y = m_roomsY - 1;
            finished = true;
        }

        if (!finished && y > 0 && m_rooms[y - 1][x].active)
            m_rooms[y - 1][x].type = 2; // drop room
    }

    m_rooms[startY][startX].color = sf::Color::Green; // start
    m_rooms[y][x].color = sf::Color::Red;             // exit

    // Fill unused cells randomly with filler rooms
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            if (!m_rooms[yy][xx].active && std::rand() % 3 == 0)
            {
                m_rooms[yy][xx].active = true;
                m_rooms[yy][xx].color = sf::Color(80, 80, 80);
                m_rooms[yy][xx].type = 0;
            }
        }
    }

 
    // Assign exits between adjacent active rooms
    for (int yy = 0; yy < m_roomsY; ++yy)
    {
        for (int xx = 0; xx < m_roomsX; ++xx)
        {
            if (!m_rooms[yy][xx].active)
                continue;

            // Right neighbor
            if (xx < m_roomsX - 1 && m_rooms[yy][xx + 1].active)
            {
                m_rooms[yy][xx].exitRight = true;
                m_rooms[yy][xx + 1].exitLeft = true;
            }

            // Down neighbor
            if (yy < m_roomsY - 1 && m_rooms[yy + 1][xx].active)
            {
                m_rooms[yy][xx].exitDown = true;
                m_rooms[yy + 1][xx].exitUp = true;
            }
        }
    }

    //Generate interior tile map for each active room
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
    int width = Room::width;
    int height = Room::height;

    room.tiles.resize(height, std::vector<int>(width, FLOOR));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
                room.tiles[i][j] = WALL;
            else
                room.tiles[i][j] = (rand() % 100 < 20) ? WALL : FLOOR;
        }
    }

    int mid = width / 2;

    // Up
    if (room.exitUp)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int j = mid + dx;
            if (j >= 0 && j < width)
                room.tiles[0][j] = FLOOR, room.tiles[1][j] = FLOOR;
        }
    }

    // Down
    if (room.exitDown)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int j = mid + dx;
            if (j >= 0 && j < width)
                room.tiles[height - 1][j] = FLOOR, room.tiles[height - 2][j] = FLOOR;
        }
    }

    // Left
    if (room.exitLeft)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int i = mid + dy;
            if (i >= 0 && i < height)
                room.tiles[i][0] = FLOOR, room.tiles[i][1] = FLOOR;
        }
    }

    // Right
    if (room.exitRight)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int i = mid + dy;
            if (i >= 0 && i < height)
                room.tiles[i][width - 1] = FLOOR, room.tiles[i][width - 2] = FLOOR;
        }
    }
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

