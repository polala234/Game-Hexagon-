#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "SFML/Audio/SoundBuffer.hpp"

struct GameRecord { //game score of both players + timestamp
    int p1{}, p2{};
    std::string timestamp;
};

class MainMenu {
public:
    MainMenu(const sf::Font &font);

    ~MainMenu();

    void draw(sf::RenderWindow &window);
    int handleClick(const sf::Vector2f &mousePos) const;
    void saveRanking(const std::string& filename, int p1, int p2);
    void loadRanking();
    sf::Texture logoTexture;
    sf::Sprite logoSprite;


private:
    std::vector<GameRecord> topGames;
    sf::Font font;
    sf::RectangleShape twoPlayerButton;
    sf::RectangleShape vsAIButton;
    sf::RectangleShape continueButton;
    sf::Text twoPlayerText;
    sf::Text vsAIText;
    sf::Text continueText;
    sf::Text topGameText;

};
