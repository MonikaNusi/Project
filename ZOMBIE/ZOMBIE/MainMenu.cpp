#include "MainMenu.h"
#include <iostream>

MainMenu::MainMenu()
{
    if (!m_font.loadFromFile("ASSETS/FONTS/zombiepixel.ttf"))
    {
        std::cout << "Failed to load menu font\n";
    }
 
    if (!m_backgroundTexture.loadFromFile("ASSETS/IMAGES/bgg.png"))
    {
        std::cout << "Failed to load menu background\n";
    }
    else
    {
        m_backgroundTexture.setSmooth(true);
        m_backgroundSprite.setTexture(m_backgroundTexture);
    }
    
    // Setup title text
    m_titleText.setFont(m_font);
    m_titleText.setString("ZOMBIE");
    m_titleText.setCharacterSize(100);
    m_titleText.setFillColor(sf::Color(200, 200, 200));
  
    m_playText.setFont(m_font);
    m_playText.setString("PLAY");
    m_playText.setCharacterSize(60);
    m_playText.setFillColor(sf::Color(200, 200, 200));
    
    m_exitText.setFont(m_font);
    m_exitText.setString("EXIT");
    m_exitText.setCharacterSize(60);
    m_exitText.setFillColor(sf::Color(200, 200, 200));
}

MainMenu::MenuResult MainMenu::show(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float centerX = windowSize.x / 2.f;
    float centerY = windowSize.y / 2.f;
    
    sf::Vector2u bgSize = m_backgroundTexture.getSize();
    if (bgSize.x > 0 && bgSize.y > 0)
    {
        float scaleX = static_cast<float>(windowSize.x) / bgSize.x;
        float scaleY = static_cast<float>(windowSize.y) / bgSize.y;
        m_backgroundSprite.setScale(scaleX, scaleY);
    }
 
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    m_titleText.setPosition(centerX, centerY - 200.f);
    
    sf::FloatRect playBounds = m_playText.getLocalBounds();
    m_playText.setOrigin(playBounds.width / 2.f, playBounds.height / 2.f);
    m_playText.setPosition(centerX, centerY);
    
    sf::FloatRect exitBounds = m_exitText.getLocalBounds();
    m_exitText.setOrigin(exitBounds.width / 2.f, exitBounds.height / 2.f);
    m_exitText.setPosition(centerX, centerY + 120.f);
    
    m_result = MenuResult::Nothing;
    
    while (m_result == MenuResult::Nothing)
    {
        handleInput(window);
        update(window);
        render(window);
    }
    
    return m_result;
}

void MainMenu::handleInput(sf::RenderWindow& window)
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_result = MenuResult::Exit;
            window.close();
        }
        
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                m_result = MenuResult::Exit;
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                
                if (m_playText.getGlobalBounds().contains(mousePosF))
                {
                    m_result = MenuResult::Play;
                }
                
                if (m_exitText.getGlobalBounds().contains(mousePosF))
                {
                    m_result = MenuResult::Exit;
                }
            }
        }
    }
}

void MainMenu::update(sf::RenderWindow& window)
{

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    if (m_playText.getGlobalBounds().contains(mousePosF))
    {
        m_playHovered = true;
        m_playText.setFillColor(sf::Color(100, 100, 100));
    }
    else
    {
        m_playHovered = false;
        m_playText.setFillColor(sf::Color(200, 200, 200));
    }
    
    if (m_exitText.getGlobalBounds().contains(mousePosF))
    {
        m_exitHovered = true;
        m_exitText.setFillColor(sf::Color(100, 100, 100));
    }
    else
    {
        m_exitHovered = false;
        m_exitText.setFillColor(sf::Color(200, 200, 200));
    }
}

void MainMenu::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(20, 20, 20));
    
    window.draw(m_backgroundSprite);
    window.draw(m_titleText);
    
    window.draw(m_playText);
    window.draw(m_exitText);
    
    window.display();
}
