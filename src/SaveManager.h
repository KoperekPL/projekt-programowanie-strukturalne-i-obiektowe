#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "GameObject.h"
#include "WaveManager.h"
#include "Map.h"

class SaveManager {
public:
    static void saveGame(const std::string& saveFilePath,
                         const std::vector<std::shared_ptr<GameObject>>& objects,
                         int playerHp, int maxPlayerHp, int baseHp, int maxBaseHp,
                         int money, float timeScale, const WaveManager& waveManager,
                         const std::string& mapPath);

    static bool loadGame(const std::string& saveFilePath,
                         std::vector<std::shared_ptr<GameObject>>& objects,
                         int& playerHp, int& maxPlayerHp, int& baseHp, int& maxBaseHp,
                         int& money, float& timeScale, WaveManager& waveManager,
                         std::string& mapPath);
};

#endif
