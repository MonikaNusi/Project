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

    // Torch texture
    if (!m_torchTexture.loadFromFile("ASSETS/IMAGES/torch.png"))
    {
        std::cout << "Failed to load torch texture for menu\n";
    }
    m_torchTexture.setSmooth(false);

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

    m_difficultyLabel.setFont(m_font);
    m_difficultyLabel.setString("DIFFICULTY");
    m_difficultyLabel.setCharacterSize(40);
    m_difficultyLabel.setFillColor(sf::Color(180, 180, 180));

    m_easyText.setFont(m_font);
    m_easyText.setString("EASY");
    m_easyText.setCharacterSize(40);
    m_easyText.setFillColor(sf::Color(200, 200, 200));

    m_mediumText.setFont(m_font);
    m_mediumText.setString("MEDIUM");
    m_mediumText.setCharacterSize(40);
    m_mediumText.setFillColor(sf::Color(200, 200, 200));

    m_hardText.setFont(m_font);
    m_hardText.setString("HARD");
    m_hardText.setCharacterSize(40);
    m_hardText.setFillColor(sf::Color(200, 200, 200));
}

void MainMenu::setupTorches(const sf::Vector2u& windowSize)
{
    m_torches.clear();
    float centerX = windowSize.x / 2.f;
    float centerY = windowSize.y / 2.f;

    std::vector<sf::Vector2f> positions = {
        {windowSize.x * 1.f / 3.6f, centerY - 250.f},  
        {centerX,                  centerY - 250.f}, 
        {windowSize.x * 5.f / 6.f, centerY - 250.f}    
    };

    for (auto& pos : positions)
    {
        Torch torch(pos);
        torch.sprite.setTexture(m_torchTexture);
        torch.sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(m_torchFrameWidth), static_cast<int>(m_torchFrameHeight)));
        torch.sprite.setOrigin(m_torchFrameWidth / 2.f, m_torchFrameHeight / 2.f);
        torch.sprite.setPosition(pos);
        torch.sprite.setScale(4.f, 4.f);
        torch.currentFrame = rand() % m_torchFrameCount;
        torch.frameTimer = static_cast<float>(rand() % 100) / 100.f * m_torchFrameTime;
        m_torches.push_back(torch);
    }
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

    setupTorches(windowSize);

    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    m_titleText.setPosition(centerX, centerY - 250.f);

    sf::FloatRect playBounds = m_playText.getLocalBounds();
    m_playText.setOrigin(playBounds.width / 2.f, playBounds.height / 2.f);
    m_playText.setPosition(centerX, centerY - 80.f);

    sf::FloatRect labelBounds = m_difficultyLabel.getLocalBounds();
    m_difficultyLabel.setOrigin(labelBounds.width / 2.f, labelBounds.height / 2.f);
    m_difficultyLabel.setPosition(centerX, centerY + 20.f);

    sf::FloatRect easyBounds = m_easyText.getLocalBounds();
    m_easyText.setOrigin(easyBounds.width / 2.f, easyBounds.height / 2.f);
    m_easyText.setPosition(centerX - 180.f, centerY + 90.f);

    sf::FloatRect medBounds = m_mediumText.getLocalBounds();
    m_mediumText.setOrigin(medBounds.width / 2.f, medBounds.height / 2.f);
    m_mediumText.setPosition(centerX, centerY + 90.f);

    sf::FloatRect hardBounds = m_hardText.getLocalBounds();
    m_hardText.setOrigin(hardBounds.width / 2.f, hardBounds.height / 2.f);
    m_hardText.setPosition(centerX + 180.f, centerY + 90.f);

    sf::FloatRect exitBounds = m_exitText.getLocalBounds();
    m_exitText.setOrigin(exitBounds.width / 2.f, exitBounds.height / 2.f);
    m_exitText.setPosition(centerX, centerY + 200.f);

    m_result = MenuResult::Nothing;

    sf::Clock clock;
    while (m_result == MenuResult::Nothing)
    {
        handleInput(window);

        // Animate torches
        float dt = clock.restart().asSeconds();
        for (auto& torch : m_torches)
        {
            torch.frameTimer += dt;
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

                if (m_easyText.getGlobalBounds().contains(mousePosF))
                {
                    m_selectedDifficulty = DifficultySettings::Level::Easy;
                }

                if (m_mediumText.getGlobalBounds().contains(mousePosF))
                {
                    m_selectedDifficulty = DifficultySettings::Level::Medium;
                }

                if (m_hardText.getGlobalBounds().contains(mousePosF))
                {
                    m_selectedDifficulty = DifficultySettings::Level::Hard;
                }
            }
        }
    }
}

void MainMenu::update(sf::RenderWindow& window)
{

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    m_playHovered = m_playText.getGlobalBounds().contains(mousePosF);
    m_playText.setFillColor(m_playHovered ? sf::Color(100, 100, 100) : sf::Color(200, 200, 200));

    m_exitHovered = m_exitText.getGlobalBounds().contains(mousePosF);
    m_exitText.setFillColor(m_exitHovered ? sf::Color(100, 100, 100) : sf::Color(200, 200, 200));

    bool easySelected = (m_selectedDifficulty == DifficultySettings::Level::Easy);
    bool medSelected = (m_selectedDifficulty == DifficultySettings::Level::Medium);
    bool hardSelected = (m_selectedDifficulty == DifficultySettings::Level::Hard);

    m_easyHovered = m_easyText.getGlobalBounds().contains(mousePosF);
    if (easySelected)
        m_easyText.setFillColor(sf::Color(50, 200, 50));
    else
        m_easyText.setFillColor(m_easyHovered ? sf::Color(100, 100, 100) : sf::Color(200, 200, 200));

    m_mediumHovered = m_mediumText.getGlobalBounds().contains(mousePosF);
    if (medSelected)
        m_mediumText.setFillColor(sf::Color(200, 200, 50));
    else
        m_mediumText.setFillColor(m_mediumHovered ? sf::Color(100, 100, 100) : sf::Color(200, 200, 200));

    m_hardHovered = m_hardText.getGlobalBounds().contains(mousePosF);
    if (hardSelected)
        m_hardText.setFillColor(sf::Color(200, 50, 50));
    else
        m_hardText.setFillColor(m_hardHovered ? sf::Color(100, 100, 100) : sf::Color(200, 200, 200));
}

void MainMenu::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(20, 20, 20));
    
    window.draw(m_backgroundSprite);

    // Draw torches behind the title
    for (const auto& torch : m_torches)
    {
        window.draw(torch.sprite);
    }

  
    if (m_selectedDifficulty == DifficultySettings::Level::Hard)
    {
        sf::RectangleShape redTint(sf::Vector2f(window.getSize().x, window.getSize().y));
        redTint.setFillColor(sf::Color(120, 10, 10, 110));
        redTint.setPosition(0, 0);
        window.draw(redTint);
    }

    float panelWidth = 700.f;
    float panelHeight = 600.f;
    float centerX = window.getSize().x / 2.f;
    float centerY = window.getSize().y / 2.f + 30.f;
    sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
    panel.setOrigin(panelWidth / 2.f, panelHeight / 2.f);
    panel.setPosition(centerX, centerY);
    panel.setFillColor(sf::Color(20, 20, 30, 200));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(80, 80, 100, 180));
    window.draw(panel);

    window.draw(m_titleText);
    window.draw(m_playText);
    window.draw(m_difficultyLabel);
    window.draw(m_easyText);
    window.draw(m_mediumText);
    window.draw(m_hardText);
    window.draw(m_exitText);

    window.display();
}
