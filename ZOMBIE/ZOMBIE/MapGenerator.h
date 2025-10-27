#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class MapGenerator
{
public:

    struct Room
    {
        bool active = false;
        int type = 0;
        sf::Color color = sf::Color(50, 50, 50);
        bool exitUp = false, exitDown = false, exitLeft = false, exitRight = false;

        //interior map data
        static const int width = 10;
        static const int height = 10;
        std::vector<std::vector<int>> tiles; // 0 = floor, 1 = wall
    };

    MapGenerator(int roomsX, int roomsY, int roomSize);
    void generate();
    void render(sf::RenderWindow& window);
    struct Room;
    const Room& getRoom(int x, int y) const;

private:

    int m_roomsX;
    int m_roomsY;
    int m_roomSize;
    int m_gap = 5; // spacing between rooms

    std::vector<std::vector<Room>> m_rooms;
    sf::RectangleShape m_roomShape;

    void generateRoomLayout(Room& room);
};
