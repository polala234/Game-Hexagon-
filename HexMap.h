#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/Sound.hpp"


struct HexTile {
    sf::CircleShape hex;
    int player{};
    sf::Vector2i axialCrd; //axial coordinates q,r
    bool highlighted{};
};
class HexMap : public sf::Drawable, public sf::Transformable {
public:
    HexMap(const sf::Font &font, float hS);
    ~HexMap();

    int getPlayer1Score() const;
    int getPlayer2Score() const;
    bool loadHexMap();
    void tileChoice(const sf::Vector2f& mousePos);
    void setVsAI(bool enable);
    void setCurrentPlayer(int p);
    void playAI();
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    int checkWinner() const;
private:
    std::vector<HexTile> hexes;
    float hexSize;
    int currentPlayer;
    bool selectingSource;
    bool playVsAI;

    sf::Font font;
    sf::Color player1 = sf::Color(232,78,96,255);
    sf::Color player2 = sf::Color(123,189,201,255);
    sf::Color background = sf::Color(35, 44, 100);
    sf::Color board = sf::Color(62, 77, 173);
    sf::Vector2i selectedSourceCoord;
    sf::SoundBuffer buffer;
    sf::Sound sound;
    void draw(sf::RenderTarget& target, sf::RenderStates states)const override ;
};

