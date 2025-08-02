#include "HexMap.h"
#include <SFML/Graphics.hpp>
#include "SFML/Audio/Sound.hpp"
#include <math.h>
#include <fstream>

HexMap::HexMap(const sf::Font &font, float hS)
    : hexSize(hS), sound(buffer), currentPlayer(1), playVsAI(false) , selectingSource(false), font(font){
    //load sound
    auto path = std::filesystem::current_path();
    path = path.parent_path().parent_path();
    path /= "assets/moveSound.wav";
     if (!buffer.loadFromFile(path.string())) {
         throw std::runtime_error("Failed to load sound!");
    };
}

HexMap::~HexMap() = default;

int HexMap::getPlayer1Score() const { //return total tiles owned by p1
    int count = 0;
    for (const auto &tile: hexes) {
        if (tile.player == 1) count++;
    }
    return count;
}

int HexMap::getPlayer2Score() const { //return total tiles owned by p2
    int count = 0;
    for (const auto &tile: hexes) {
        if (tile.player == 2) count++;
    }
    return count;
}

bool HexMap::loadHexMap() {
    //load new board
    hexes.clear();
    float hexHeight = std::sqrt(3.f) * hexSize;

    int radius = 4; //let the board size be max 9x9 hexagons
    //axial coordinates q column ,r row
    for (int q = -radius; q <= radius; ++q) {
        int rMin = std::max(-radius, -q - radius);
        int rMax = std::min(radius, -q + radius);

        for (int r = rMin; r <= rMax; ++r) {
            // Convert axial to pixel coordinates source: https://www.redblobgames.com/grids/hexagons/#conversions
            float x = hexSize * 3 / 2 * q; //move horizontally
            float y = hexHeight * (r + q / 2.f); //move vertically
            //create hexagon rotated flat side up down
            HexTile tile;
            tile.hex = sf::CircleShape(hexSize, 6);
            tile.hex.setRotation(sf::degrees(30));

            tile.hex.setPosition({x + 700.f, y + 350.f}); //set position in pixel coordinates on screen

            if ((q == -4 and r == 0)    //color starting tiles of p2
                or (q == 0 and r == 4)
                or (q == 4 and r == -4)
            ) {
                tile.hex.setFillColor(player2);
                tile.player = 2;
            } else if ((q == 0 and r == -4)     //color starting tiles of p1
                       or (q == 4 and r == 0)
                       or (q == -4 and r == 4)
            ) {
                tile.hex.setFillColor(player1);
                tile.player = 1;
            } else if ((q == 0 and r == -1)        //tiles that are 'holes' in the board
                       or (q == 1 and r == 0)
                       or (q == -1 and r == 1)
            ) {
                tile.player = 3;
                tile.hex.setFillColor(background);
            } else
                tile.hex.setFillColor(board);
            tile.hex.setOutlineThickness(-2.f);
            tile.hex.setOutlineColor(background);
            tile.axialCrd = {q, r};
            hexes.push_back(tile); //add to vector
        }
    }
    return true;
}

int hexDistance(sf::Vector2i a, sf::Vector2i b) {
    //calculate axial distance between two tiles https://www.redblobgames.com/grids/hexagons/#conversions
    return (abs(a.x - b.x)
            + abs(a.x + a.y - b.x - b.y)
            + abs(a.y - b.y)) / 2;
}

void HexMap::tileChoice(const sf::Vector2f &mousePos) {
    for (auto &tile: hexes) {
        if (tile.hex.getGlobalBounds().contains(mousePos)) {
            //chosen source tile
            if (selectingSource) {
                if (tile.player == currentPlayer) {
                    selectedSourceCoord = tile.axialCrd;
                    selectingSource = false;
                    // Highlight possible moves
                    for (auto &other: hexes) {
                        if (other.player == 0) { //if its a 'free' tile
                            int dist = hexDistance(tile.axialCrd, other.axialCrd);
                            if (dist == 1 || dist == 2) { //distance can be max 2 tiles away
                                other.highlighted = true;
                                other.hex.setFillColor(sf::Color(247, 203, 106, 255));
                            }
                        }
                    }
                }
                return;
            }

            if (tile.highlighted) {
                sound.play();
                int dist = hexDistance(selectedSourceCoord, tile.axialCrd);
                //make move to chosen highlighted tile
                tile.player = currentPlayer;
                tile.hex.setFillColor(currentPlayer == 1 ? player1 : player2);
                //take over enemy adjacent tiles
                for (auto &other: hexes) {
                    if (hexDistance(tile.axialCrd, other.axialCrd) == 1) {
                        if (other.player != 0 && other.player != currentPlayer && other.player != 3) {
                            other.player = currentPlayer;
                            other.hex.setFillColor(currentPlayer == 1 ? player1 : player2);
                        }
                    }
                }
                if (dist == 2) {
                    // jump: clear original
                    for (auto &source: hexes) {
                        if (source.axialCrd == selectedSourceCoord) {
                            source.player = 0;
                            source.hex.setFillColor(board);
                            break;
                        }
                    }
                }
                //reset highlights
                for (auto &other: hexes) {
                    if (other.highlighted) {
                        other.highlighted = false;
                        if (other.player == 0) {
                            // Only reset unowned tiles back
                            other.hex.setFillColor(board);
                        }
                    }
                }
                //change to next player
                currentPlayer = currentPlayer == 1 ? 2 : 1;
                selectingSource = true;
                //when playing against ai, ai makes move
                if (playVsAI && currentPlayer == 2) {
                    playAI();
                }
            } else if (tile.player == currentPlayer) { //if the player chooses to select a different tile for the source
                // Reset previous highlights
                for (auto &other: hexes) {
                    if (other.highlighted) {
                        other.highlighted = false;
                        other.hex.setFillColor(board);
                    }
                }
                selectedSourceCoord = tile.axialCrd;
                //Highlight new
                for (auto &other: hexes) {
                    if (other.player == 0) {
                        int dist = hexDistance(tile.axialCrd, other.axialCrd);
                        if (dist == 1 || dist == 2) {
                            other.highlighted = true;
                            other.hex.setFillColor(sf::Color(247, 203, 106, 255));
                        }
                    }
                }
            }
            return;
        }
    }
}

void HexMap::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    states.transform *= getTransform();
    for (const auto &tile: hexes) {
        target.draw(tile.hex, states); //draw board
    }
    //display which player's turn it is
    sf::Text playerTurnText(font, currentPlayer == 1 ? "Player 1's Turn" : "Player 2's Turn", 30);
    playerTurnText.setFillColor(sf::Color(247, 203, 106, 255));
    playerTurnText.setPosition({30.f, 20.f});
    //display score
    int player1Score = getPlayer1Score();
    int player2Score = getPlayer2Score();
    sf::Text score(
        font, "Score:\nPlayer 1:  " + std::to_string(player1Score) + "\nPlayer 2:   " + std::to_string(player2Score),
        30);
    score.setFillColor(sf::Color(247, 203, 106, 255));
    score.setPosition({1100.f, 550.f});

    target.draw(playerTurnText);
    target.draw(score);
}

void HexMap::setVsAI(bool enable) { //set true when playing against ai
    playVsAI = enable;
}
void HexMap::setCurrentPlayer(int p) {
    currentPlayer = p;
}

void HexMap::playAI() {
    int bestScore = -1;
    sf::Vector2i bestSource;
    sf::Vector2i bestTarget;

    // Search for best move
    for (const auto &tile: hexes) {
        if (tile.player == 2) { //chose only tiles belonging to p2
            for (const auto &target: hexes) {
                if (target.player == 0) {
                    int dist = hexDistance(tile.axialCrd, target.axialCrd);
                    if (dist == 1 || dist == 2) { //consider only distance 1 or 3
                        // Count how many player1 tiles would be converted
                        int convertCount = 0;
                        for (const auto &neighbor: hexes) {
                            if (hexDistance(target.axialCrd, neighbor.axialCrd) == 1 &&
                                neighbor.player == 1) {
                                convertCount++;
                            }
                        }
                        if (convertCount > bestScore) {
                            bestScore = convertCount;
                            bestSource = tile.axialCrd;
                            bestTarget = target.axialCrd;
                        }
                    }
                }
            }
        }
    }
    // Perform move
    int dist = hexDistance(bestSource, bestTarget);
    for (auto &tile: hexes) {
        if (tile.axialCrd == bestTarget) {
            sound.play();
            tile.player = 2;
            tile.hex.setFillColor(player2);
        }
        //perform jump if distance is 2
        if (dist == 2 && tile.axialCrd == bestSource) {
            tile.player = 0;
            tile.hex.setFillColor(board);
        }
        // Flip adjacent player1 tiles
        if (hexDistance(bestTarget, tile.axialCrd) == 1 &&
            tile.player == 1) {
            tile.player = 2;
            tile.hex.setFillColor(player2);
        }
    }
    currentPlayer = 1; //go back to p1
    selectingSource = true;
}

void HexMap::saveToFile(const std::string &filename) const { //when quitting save game
    std::fstream file(filename, std::ios::out | std::ios::trunc);
    if (file) {
        file << playVsAI << ' ' << currentPlayer << '\n'; //info if game was player vs player or ai and who current player is
        //save info about board
        for (const auto &tile: hexes) {
            file << tile.axialCrd.x << ' '
                    << tile.axialCrd.y << ' '
                    << tile.player << '\n';
        }
    }else throw std::runtime_error("Failed to open file for writing");
}

void HexMap::loadFromFile(const std::string &filename) {
    //open previous game from file
    std::fstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    file >> playVsAI >> currentPlayer;
    hexes.clear();

    float hexHeight = std::sqrt(3.f) * hexSize;
    int q, r, player;

    while (file >> q >> r >> player) {
        float x = hexSize * 3 / 2 * q;
        float y = hexHeight * (r + q / 2.f);
        //load board, tiles coordinates and adequate color
        HexTile tile;
        tile.hex = sf::CircleShape(hexSize, 6);
        tile.hex.setRotation(sf::degrees(30));
        tile.hex.setPosition({x + 700.f, y + 350.f});
        tile.player = player;
        tile.axialCrd = {q, r};
        tile.hex.setOutlineThickness(-2.f);
        tile.hex.setOutlineColor(background);

        if (player == 1) tile.hex.setFillColor(player1);
        else if (player == 2) tile.hex.setFillColor(player2);
        else if (player == 3) tile.hex.setFillColor(background);
        else tile.hex.setFillColor(board);

        hexes.push_back(tile);
    }
    selectingSource = true;
}

int HexMap::checkWinner() const {
    int p1 = getPlayer1Score();
    int p2 = getPlayer2Score();
    //if one of the players has no more tiles
    if (p1 == 0) return 2; // Player 2 wins
    if (p2 == 0) return 1; // Player 1 wins

    bool hasMoveP1 = false;
    bool hasMoveP2 = false;
    //check if a player is blocked and has no more moves
    for (const auto &tile: hexes) {
        if (tile.player == 1 || tile.player == 2) {
            for (const auto &target: hexes) {
                if (target.player == 0) {
                    int dist = hexDistance(tile.axialCrd, target.axialCrd);
                    if (dist == 1 || dist == 2) {
                        if (tile.player == 1) hasMoveP1 = true;
                        if (tile.player == 2) hasMoveP2 = true;
                    }
                }
            }
        }
    }

    if (!hasMoveP1 && !hasMoveP2) { //both have no more moves
        if (p1 == p2) return 0; //draw
        return (p1 > p2 ? 1 : 2); //winner that has more tiles
    }
    if (!hasMoveP1) return 2; //only p1 has no more moves
    if (!hasMoveP2) return 3; //only p2 has no more moves

    return 0;
}
