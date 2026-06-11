#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>
#include "Map.h"
#include "Enemy.h"
#include "Tower.h"
#include "AssetManager.h"

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
    int playerFrame;
    float playerAnimTime;
    float playerSpeed;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Tower> towers;
    
    AssetManager assets;
    float towerAnimTime;
    int towerAnimFrame;

    std::map<TowerType, TowerStats> towerConfigs;

    bool showPopup;
    std::string popupText;
    Tower* selectedTower;
    Tower* hoveredTower;

    sf::RectangleShape popupBg;
    sf::RectangleShape closeBtn;
    sf::RectangleShape cross1;
    sf::RectangleShape cross2;

    sf::Font font;
    bool hasFont;
    sf::Text text;
    sf::Text moneyText;

    std::vector<sf::RectangleShape> levelBtns;
    std::vector<sf::Text> levelTexts;

    int money;
    bool debugMode;
    
    int baseHp;
    int maxBaseHp;
    int playerHp;
    int maxPlayerHp;

    std::map<std::string, EnemyStats> enemyConfigs;
};

#endif
