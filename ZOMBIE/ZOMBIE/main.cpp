/// <summary>
/// @description 
/// 
/// @author Adam Sharpe
/// @date
/// </summary>

#ifdef _DEBUG 
#pragma comment(lib,"sfml-graphics-d.lib") 
#pragma comment(lib,"sfml-audio-d.lib") 
#pragma comment(lib,"sfml-system-d.lib") 
#pragma comment(lib,"sfml-window-d.lib") 
#pragma comment(lib,"sfml-network-d.lib") 
#else 
#pragma comment(lib,"sfml-graphics.lib") 
#pragma comment(lib,"sfml-audio.lib") 
#pragma comment(lib,"sfml-system.lib") 
#pragma comment(lib,"sfml-window.lib") 
#pragma comment(lib,"sfml-network.lib") 
#endif 


#include "Game.h"
#include "MainMenu.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode(1200U, 1000U, 32U), "SFML Game");
	
	while (window.isOpen())
	{
		MainMenu menu;
		MainMenu::MenuResult result = menu.show(window);
		
		if (result == MainMenu::MenuResult::Exit)
		{
			break;
		}
		
		if (result == MainMenu::MenuResult::Play)
		{
			Game game;
			game.run();
		}
	}
	
	return 0;
}