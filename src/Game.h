#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <map>
#include "Map.h"
#include "Enemy.h"
#include "Tower.h"
#include "AssetManager.h"
#include "GameObject.h"
#include "WaveManager.h"

enum class GameState {
    MainMenu,
    Playing,
    Paused,
    GameOver
};

class Game {
public:
    Game(const std::string& name, const std::string& mapPath, bool isNewGame);
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void handlePopupClick(float mx, float my);
    
    void renderPauseMenu();
    void processPauseEvents(sf::Event& event);

    GameState currentState = GameState::Playing;
    std::string currentSaveName;
    
    sf::RenderWindow window;
    bool isFullscreen;
    sf::Clock clock;
    Map map;
    std::string currentMapPath;
    
    sf::Music bgMusic;
    float sfxVolume = 100.f;
    float musicVolume = 50.f;

    struct Laser {
        sf::Vector2f start;
        sf::Vector2f end;
        sf::Color color;
        float lifeTime = 0.2f;
    };
    std::vector<Laser> activeLasers;
    
    sf::Sprite player;
    int playerFrame;
    float playerAnimTime;
    float playerSpeed;
    
    bool hasSword = false;
    float meleeCooldownTimer = 0.f;

    struct AttackSlash {
        sf::Vector2f position;
        float angle;
        float lifeTime;
        bool isRight;
    };
    std::vector<AttackSlash> attackSlashes;

    std::vector<std::shared_ptr<GameObject>> gameObjects;
    
    AssetManager assets;
    float towerAnimTime;
    int towerAnimFrame;

    std::map<TowerType, TowerStats> towerConfigs;

    bool showPopup;
    bool showShop = false;
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
    sf::Text waveCounterText;

    int money;
    bool debugMode;
    
    int playerHp;
    int maxPlayerHp;
    int baseHp;
    int maxBaseHp;

    float timeScale = 1.0f;
    float slowdownTimer = 0.0f;
    
    float autoSaveTimer = 0.0f;
    float saveFeedbackTimer = 0.0f;

    WaveManager waveManager;

    std::map<std::string, EnemyStats> enemyConfigs;
};

#endif
// End of Game.h
