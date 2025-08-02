#include "MainMenu.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ranges>
#include <filesystem>
#include <iostream>

#include "fmt/os.h"

MainMenu::MainMenu(const sf::Font &font): logoSprite(logoTexture), twoPlayerText(font, "Player vs Player", 30),
                                          vsAIText(font, "Player vs Computer", 30),
                                          continueText(font, "Continue Game", 30),
                                          topGameText(font, "Top Scores", 30),
                                          font(font) {
    // logo
    auto logoPath = std::filesystem::current_path();
    logoPath = logoPath.parent_path().parent_path();
    logoPath /= "assets/logo.png";
    std::cout << "Logo Path: " << logoPath << '\n';


    if (!logoTexture.loadFromFile(logoPath.string())) {
        throw std::runtime_error("Failed to load logo!");
    }

    logoSprite = sf::Sprite(logoTexture);
    logoSprite.setPosition({550.f, 50.f});
    logoSprite.setScale({0.4f, 0.4f});
    loadRanking(); //reload ranking when opening main menu

    //main menu buttons
    float buttonX = 250.f;

    twoPlayerButton.setSize({450.f, 70.f});
    twoPlayerButton.setPosition({buttonX, 400.f});
    twoPlayerButton.setFillColor(sf::Color(247, 203, 106, 255));

    vsAIButton.setSize({450.f, 70.f});
    vsAIButton.setPosition({buttonX, 500.f});
    vsAIButton.setFillColor(sf::Color(247, 203, 106, 255));

    continueButton.setSize({450.f, 70.f});
    continueButton.setPosition({buttonX, 600.f});
    continueButton.setFillColor(sf::Color(247, 203, 106, 255));

    twoPlayerText.setPosition({buttonX + 70.f, 415.f});
    twoPlayerText.setFillColor(sf::Color(35, 44, 100));

    vsAIText.setPosition({buttonX + 70.f, 515.f});
    vsAIText.setFillColor(sf::Color(35, 44, 100));

    continueText.setPosition({buttonX + 70.f, 615.f});
    continueText.setFillColor(sf::Color(35, 44, 100));

    topGameText.setFillColor(sf::Color(247, 203, 106, 255));
    topGameText.setPosition({800.f, 400.f});
}

MainMenu::~MainMenu() = default;

void MainMenu::loadRanking() {
    //refresh ranking
    topGames.clear();
    auto path = std::filesystem::current_path();
    path = path.parent_path().parent_path();
    path /= "assets/ranking.txt";
    std::ifstream in(path.string());
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream isS(line);
        GameRecord g;
        isS >> g.p1 >> g.p2;
        std::getline(isS, g.timestamp);
        topGames.push_back(g);
    }

    std::ranges::sort(topGames, [](const auto &a, const auto &b) {
        return std::max(a.p1, a.p2) > std::max(b.p1, b.p2);
    }); //sort according to a top scor of one of the players
}

void MainMenu::draw(sf::RenderWindow &window) {
    //draw main menu
    window.draw(logoSprite);
    window.draw(twoPlayerButton);
    window.draw(vsAIButton);
    window.draw(continueButton);
    window.draw(twoPlayerText);
    window.draw(vsAIText);
    window.draw(continueText);
    window.draw(topGameText);

    //display top 3 scores from ranking
    for (int i = 0; i < std::min(3, (int)topGames.size()); ++i) {
        sf::Text entry(font,
                       std::to_string(i + 1) + ". P1: " + std::to_string(topGames[i].p1) +
                       " P2: " + std::to_string(topGames[i].p2) +
                       "\n" + topGames[i].timestamp,
                       20);
        entry.setPosition({800.f, 450.f + i * 80.f});
        entry.setFillColor(sf::Color(247, 203, 106, 255));
        window.draw(entry);
    }
}

int MainMenu::handleClick(const sf::Vector2f &mousePos) const {
    //decide which button was clicked
    if (twoPlayerButton.getGlobalBounds().contains(mousePos)) return 1;
    if (vsAIButton.getGlobalBounds().contains(mousePos)) return 2;
    if (continueButton.getGlobalBounds().contains(mousePos)) return 3;
    return 0;
}

void MainMenu::saveRanking(const std::string& filename, int p1, int p2) {
    //save score to ranking file
    std::ofstream rankOut(filename, std::ios::app);
    if (rankOut) {
        std::time_t now = std::time(nullptr);
        rankOut << p1 << ' ' << p2 << ' ' << std::put_time(std::localtime(&now), "%F %T") << '\n';
    }
    //reload the rankings
    loadRanking();
}
