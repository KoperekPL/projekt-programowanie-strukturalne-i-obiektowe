#include "SaveManager.h"
#include "Tower.h"
#include "Enemy.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void SaveManager::saveGame(const std::string& saveFilePath,
                           const std::vector<std::shared_ptr<GameObject>>& objects,
                           int playerHp, int maxPlayerHp, int baseHp, int maxBaseHp,
                           int money, float timeScale, const WaveManager& waveManager,
                           const std::string& mapPath) {
    fs::path p(saveFilePath);
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    if (ec) {
        std::cerr << "Warning: create_directories failed: " << ec.message() << "\n";
    }

    std::ofstream out(saveFilePath);
    if (!out) {
        std::cerr << "Failed to open save file for writing: " << saveFilePath << "\n";
        return;
    }

    out << "MAP " << mapPath << "\n";
    out << "PLAYER_HP " << playerHp << " " << maxPlayerHp << "\n";
    out << "BASE_HP " << baseHp << " " << maxBaseHp << "\n";
    out << "MONEY " << money << "\n";
    out << "TIMESCALE " << timeScale << "\n";
    out << "WAVE " << waveManager.currentWave << "\n";
    
    // We don't save everything yet, just the basic stats for proof of concept.
    // Saving enemies and towers requires more complex serialization, but let's write basic object counts.
    
    int towerCount = 0;
    int enemyCount = 0;
    for (auto& obj : objects) {
        if (dynamic_cast<Tower*>(obj.get())) towerCount++;
        else if (dynamic_cast<Enemy*>(obj.get())) enemyCount++;
    }

    out << "TOWERS " << towerCount << "\n";
    for (auto& obj : objects) {
        if (auto t = dynamic_cast<Tower*>(obj.get())) {
            out << static_cast<int>(t->type) << " " << static_cast<int>(t->state) << " "
                << t->position.x << " " << t->position.y << " "
                << t->damageUpgradeLevel << " " << t->speedUpgradeLevel << " " << t->targetsUpgradeLevel << " "
                << t->hasTimedUpgrade << " " << t->timedUpgradeTimer << " " << t->cooldownTimer << "\n";
        }
    }

    // Enemies are omitted for simplicity of save state, usually loading an existing save removes existing mid-wave enemies and restarts the wave, 
    // or we can just save their positions. For this version, we will just save towers.
    out << "ENEMIES 0\n"; 

    out.close();
}

bool SaveManager::loadGame(const std::string& saveFilePath,
                           std::vector<std::shared_ptr<GameObject>>& objects,
                           int& playerHp, int& maxPlayerHp, int& baseHp, int& maxBaseHp,
                           int& money, float& timeScale, WaveManager& waveManager,
                           std::string& mapPath) {
    std::ifstream in(saveFilePath);
    if (!in) {
        std::cerr << "Failed to open save file for reading: " << saveFilePath << "\n";
        return false;
    }

    std::string key;
    while (in >> key) {
        if (key == "MAP") {
            std::getline(in >> std::ws, mapPath);
        } else if (key == "PLAYER_HP") {
            in >> playerHp >> maxPlayerHp;
        } else if (key == "BASE_HP") {
            in >> baseHp >> maxBaseHp;
        } else if (key == "MONEY") {
            in >> money;
        } else if (key == "TIMESCALE") {
            in >> timeScale;
        } else if (key == "WAVE") {
            int wave;
            in >> wave;
            waveManager.startWave(wave);
        } else if (key == "TOWERS") {
            int towerCount;
            in >> towerCount;
            for (int i = 0; i < towerCount; ++i) {
                int type, state, hasTimed;
                float px, py, timedTimer, cdTimer;
                int dmgLvl, spdLvl, tgtLvl;
                in >> type >> state >> px >> py >> dmgLvl >> spdLvl >> tgtLvl >> hasTimed >> timedTimer >> cdTimer;
                
                auto t = std::make_shared<Tower>();
                t->type = static_cast<TowerType>(type);
                t->state = static_cast<TowerState>(state);
                t->position = sf::Vector2f(px, py);
                t->damageUpgradeLevel = dmgLvl;
                t->speedUpgradeLevel = spdLvl;
                t->targetsUpgradeLevel = tgtLvl;
                t->hasTimedUpgrade = hasTimed;
                t->timedUpgradeTimer = timedTimer;
                t->cooldownTimer = cdTimer;
                objects.push_back(t);
            }
        } else if (key == "ENEMIES") {
            int enemyCount;
            in >> enemyCount;
            // Ignore enemies for now
        }
    }

    in.close();
    return true;
}
