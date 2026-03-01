#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class MapGenerator
{
public:

    struct Room
    {
        enum class RoomType { Empty, Normal, Treasure, Trap, Boss, Start };

        bool active = false;
        RoomType type = RoomType::Empty;

        sf::Color color = sf::Color(50, 50, 50);
        bool exitUp = false, exitDown = false, exitLeft = false, exitRight = false;

        //interior map data
        static const int width = 10;
        static const int height = 10;
        std::vector<std::vector<int>> tiles; // 0 = floor, 1 = wall, 2 = locked door
    };

    MapGenerator(int roomsX, int roomsY, int roomSize);
    void generate();
    void render(sf::RenderWindow& window);
    struct Room;
    const Room& getRoom(int x, int y) const;

    const sf::Texture& getWallTexture() const { return m_wallTexture; }
    const sf::Texture& getFloorTexture() const { return m_floorTexture; }
    const sf::Texture& getTilesetTexture() const { return m_tilesetTexture; }

    // Runtime tile modification
    void setTile(int roomX, int roomY, int tileX, int tileY, int value);

  
    sf::IntRect getTileTextureRect(const Room& room, int tileX, int tileY) const;

private:

    int m_roomsX;
    int m_roomsY;
    int m_roomSize;
    int m_gap = 5; // spacing between rooms

    sf::Texture m_wallTexture;
    sf::Texture m_floorTexture;
    sf::Texture m_tilesetTexture;  

    bool isPathValid(const sf::Vector2i& start, const sf::Vector2i& goal) const;

    std::vector<std::vector<Room>> m_rooms;
    sf::RectangleShape m_roomShape;

    void generateRoomLayout(Room& room);
    
   
    bool isWall(const Room& room, int x, int y) const;
    int countNeighborWalls(const Room& room, int x, int y) const;
};
