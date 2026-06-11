#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>
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
    void loadTowerConfig(const std::string& filepath);

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
    // upgrade-screen textures (static, shown during build indicator)
    sf::Texture towerSniperLvl1Tex;
    sf::Texture towerSniperLvlMaxTex;
    sf::Texture towerMultishotLvl1Tex;
    sf::Texture towerMultishotLvlMaxTex;
    // idle animation textures for built upgraded towers
    sf::Texture towerSniperLvl1IdleTex;
    sf::Texture towerSniperLvlMaxIdleTex;
    sf::Texture towerMultishotLvl1IdleTex;
    sf::Texture towerMultishotLvlMaxIdleTex;
    float towerAnimTime;
    int towerAnimFrame;
    sf::Texture enemyTexD;
    sf::Texture enemyTexS;
    sf::Texture enemyTexU;

    std::map<TowerType, TowerStats> towerConfigs;

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
    void loadEnemyConfig(const std::string& filepath);
};

#endif
