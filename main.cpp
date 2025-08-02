#include <SFML/Graphics.hpp>
#include "SFML/Audio/SoundBuffer.hpp"
#include <filesystem>
#include "HexMap.h"
#include "MainMenu.h"


int main() {
    //render window
    auto window = sf::RenderWindow(sf::VideoMode({2000, 1500}), "SFML",
                                   sf::Style::Default, sf::State::Windowed,
                                   sf::ContextSettings{.antiAliasingLevel = 8});
    window.setFramerateLimit(60);

    //---------------PARAMETERS---------------
    //load font
    sf::Font font;
    auto path = std::filesystem::current_path();
    path = path.parent_path().parent_path();
    path /= "assets/Orbitron-VariableFont_wght.ttf";
    if (!font.openFromFile(path.string())) {
        throw std::runtime_error("Failed to load font!");
    }

    bool isMainMenu = true; //begin with main menu
    auto hex_map = HexMap(font, 45);
    hex_map.loadHexMap();

    MainMenu menu(font);

    //quit button during game
    sf::RectangleShape quitButton({150.f, 50.f});
    quitButton.setPosition({1100.f, 690.f});
    quitButton.setFillColor(sf::Color(247, 203, 106, 255));

    sf::Text quitText(font, "QUIT", 30);
    quitText.setPosition({1130.f, 700.f});
    quitText.setFillColor(sf::Color(35, 44, 100));

    //-----------------EVENTS - main loop---------------------------------
    while (window.isOpen()) {
        //close window
        while (auto const event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            //clicks
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            //main menu window
            if (isMainMenu && event->is<sf::Event::MouseButtonPressed>()) {
                //chceck which button was clicked
                switch (menu.handleClick(mousePos)) {
                    //first button player vs player
                    case 1:
                        isMainMenu = false;
                        hex_map.setVsAI(false);
                        hex_map.loadHexMap(); //load map in player vs player mode
                        break;
                    //2nd button player vs comp
                    case 2:
                        isMainMenu = false;
                        hex_map.setVsAI(true);
                        hex_map.loadHexMap(); //load map in player vs ai mode
                        break;
                    //3rd button continue game
                    case 3: {
                        isMainMenu = false;
                        auto path = std::filesystem::current_path();
                        path = path.parent_path().parent_path();
                        path /= "assets/savegame.txt";
                        hex_map.loadFromFile(path.string()); //load previously saved game and continue
                        break;
                    }
                    default:
                        break;
                }
            } else if (!isMainMenu) {
                //while in game mode
                if (event->is<sf::Event::MouseButtonPressed>()) {
                    if (quitButton.getGlobalBounds().contains(mousePos)) {
                        //when quit button is clicked
                        auto path = std::filesystem::current_path();
                        path = path.parent_path().parent_path();
                        path /= "assets/savegame.txt";
                        hex_map.saveToFile(path.string()); //save game to be able to continue later
                        auto path2 = path.parent_path()/ "ranking.txt";
                        int p1 = hex_map.getPlayer1Score();
                        int p2 = hex_map.getPlayer2Score();
                        menu.saveRanking(path2.string(), p1, p2); //save score
                        menu.loadRanking(); //update ranking
                        hex_map.setCurrentPlayer(1);
                        isMainMenu = true; //switch back to main menu
                    } else {
                        hex_map.tileChoice(mousePos); //if clicked on board, allow player move

                        int winner = hex_map.checkWinner(); //check if game is finished
                        if (winner != 0) {
                            //if game is finished display winner announcement
                            sf::Text winnerText(font);
                            winnerText.setCharacterSize(50);
                            switch (winner) {
                                case 1: winnerText.setString("Player 1 Wins!");
                                    break;
                                case 2: winnerText.setString("Player 2 Wins!");
                                    break;
                                case 3: winnerText.setString("It's a draw!");
                                    break;
                                default: ;
                            }

                            winnerText.setFillColor(sf::Color(247, 203, 106, 255));
                            winnerText.setPosition({600.f, 200.f});
                            window.draw(winnerText);
                            window.display();
                            sf::sleep(sf::seconds(3));
                            //wait 3s and go back to main menu
                            isMainMenu = true;
                        }
                    }
                }
            }
        }

        //-----------------DRAW-------------------
        window.clear(sf::Color(35, 44, 100));
        //display main menu or game board
        if (isMainMenu) {
            menu.draw(window);
        } else {
            window.draw(hex_map);
            window.draw(quitButton);
            window.draw(quitText);
        }

        window.display();
    }
    return 0;
}
