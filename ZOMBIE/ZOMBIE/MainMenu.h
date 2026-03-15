#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class MainMenu
{
public:
    enum class MenuResult { Nothing, Play, Exit };

    MainMenu();
    
    MenuResult show(sf::RenderWindow& window);
    
private:
    void handleInput(sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void render(sf::RenderWindow& window);
    
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_playText;
    sf::Text m_exitText;
    
    sf::Texture m_backgroundTexture;
    sf::Sprite m_backgroundSprite;
    
    bool m_playHovered = false;
    bool m_exitHovered = false;
    
    MenuResult m_result = MenuResult::Nothing;
};

