#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Map.h"
#include "Enemy.h"
#include "Tower.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void handlePopupClick(float mx, float my);

    sf::RenderWindow window;
    bool isFullscreen;
    sf::Clock clock;

    Map map;
    sf::Sprite player;
    sf::Texture playerTexture;
    int playerFrame;
    float playerAnimTime;
    float playerSpeed;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Tower> towers;
    sf::Texture towerUnbuiltTex;
    sf::Texture towerLvl1Tex;
    sf::Texture towerLvl2Tex;
    sf::Texture towerLvl3Tex;
    float towerAnimTime;
    int towerAnimFrame;
    sf::Texture enemyTexD;
    sf::Texture enemyTexS;
    sf::Texture enemyTexU;

    bool showPopup;
    std::string popupText;
    Tower* selectedTower;

    sf::RectangleShape popupBg;
    sf::RectangleShape closeBtn;
    sf::RectangleShape cross1;
    sf::RectangleShape cross2;

    sf::Font font;
    bool hasFont;
    sf::Text text;

    std::vector<sf::RectangleShape> levelBtns;
    std::vector<sf::Text> levelTexts;
};

#endif
