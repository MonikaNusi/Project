#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "dificultySetting.h"


class MainMenu
{
public:
    enum class MenuResult { Nothing, Play, Exit };

    MainMenu();
    
    MenuResult show(sf::RenderWindow& window);
    DifficultySettings::Level getSelectedDifficulty() const { return m_selectedDifficulty; }

    int tryDealDamage(const sf::FloatRect& playerHitbox);
 

private:
    void handleInput(sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void render(sf::RenderWindow& window);
    
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_playText;
    sf::Text m_exitText;

    sf::Text m_easyText;
    sf::Text m_mediumText;
    sf::Text m_hardText;
    sf::Text m_difficultyLabel;
    
    sf::Texture m_backgroundTexture;
    sf::Sprite m_backgroundSprite;

    // Torch animation
    struct Torch
    {
        sf::Vector2f pos;
        sf::Sprite sprite;
        int currentFrame{ 0 };
        float frameTimer{ 0.f };

        Torch() = default;
        Torch(sf::Vector2f p) : pos(p) {}
    };

    std::vector<Torch> m_torches;
    sf::Texture m_torchTexture;
    const int m_torchFrameCount{ 3 };
    const float m_torchFrameWidth{ 32.f };
    const float m_torchFrameHeight{ 32.f };
    const float m_torchFrameTime{ 0.15f };

    bool m_playHovered = false;
    bool m_exitHovered = false;
    bool m_easyHovered = false;
    bool m_mediumHovered = false;
    bool m_hardHovered = false;

    DifficultySettings::Level m_selectedDifficulty{ DifficultySettings::Level::Medium };
    MenuResult m_result = MenuResult::Nothing;

    void setupTorches(const sf::Vector2u& windowSize);
};

