#include "Config.h"
#include <fstream>
#include <iostream>

namespace Config {

void loadEnemyConfig(const std::string& filepath, std::map<std::string, EnemyStats>& enemyConfigs) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to load enemy config: " << filepath << std::endl;
        return;
    }

    std::string line;
    std::string currentEnemy = "";
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t firstNonSpace = line.find_first_not_of(' ');
        if (firstNonSpace == std::string::npos) continue;

        if (firstNonSpace == 0) {
            currentEnemy = line;
            if (!currentEnemy.empty() && currentEnemy.back() == ':') {
                currentEnemy.pop_back();
            }
            enemyConfigs[currentEnemy] = EnemyStats();
        } else if (firstNonSpace == 2 && !currentEnemy.empty()) {
            std::string keyVal = line.substr(2);
            size_t colonPos = keyVal.find(':');
            if (colonPos != std::string::npos) {
                std::string key = keyVal.substr(0, colonPos);
                std::string valStr = keyVal.substr(colonPos + 1);
                
                size_t vStart = valStr.find_first_not_of(" \t");
                if (vStart != std::string::npos) valStr = valStr.substr(vStart);
                
                if (key == "hp") enemyConfigs[currentEnemy].maxHp = std::stoi(valStr);
                else if (key == "speed") enemyConfigs[currentEnemy].speed = std::stof(valStr);
                else if (key == "damage_castle") enemyConfigs[currentEnemy].castleDamage = std::stoi(valStr);
                else if (key == "damage_player") enemyConfigs[currentEnemy].playerDamage = std::stoi(valStr);
            }
        }
    }
}

void loadTowerConfig(const std::string& filepath, std::map<TowerType, TowerStats>& towerConfigs) {
  TowerStats baseStats;
  baseStats.cost = 50; baseStats.buildTime = 3.0f; baseStats.damage = 5; baseStats.cooldown = 1.0f; baseStats.range = 100.f;
  towerConfigs[TowerType::Base] = baseStats;

  TowerStats sniperStats;
  sniperStats.cost = 100; sniperStats.buildTime = 5.0f; sniperStats.damage = 25; sniperStats.cooldown = 2.5f; sniperStats.range = 250.f;
  towerConfigs[TowerType::Sniper] = sniperStats;

  TowerStats multiStats;
  multiStats.cost = 120; multiStats.buildTime = 4.0f; multiStats.damage = 8; multiStats.cooldown = 0.8f; multiStats.range = 120.f;
  multiStats.baseTargets = 3;
  towerConfigs[TowerType::Multishot] = multiStats;

  TowerStats healerStats;
  healerStats.cost = 1000; healerStats.buildTime = 10.0f; healerStats.damage = 0; healerStats.cooldown = 2.0f; healerStats.range = 50.f;
  towerConfigs[TowerType::Healer] = healerStats;

  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "Nie udalo sie wczytac pliku konfiguracyjnego wiez: "
              << filepath << "\n";
    return;
  }

  std::string rawLine;
  TowerType currentType = TowerType::Empty;
  std::string currentCategory = "";
  int currentLevelIndex = -1;

  while (std::getline(file, rawLine)) {
    size_t start = rawLine.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
      continue;
    int indent = static_cast<int>(start);
    std::string line = rawLine.substr(start);

    if (line[0] == '#' || line.empty())
      continue;

    if (indent == 0) {
      if (line.find("Tower_Base:") == 0) currentType = TowerType::Base;
      else if (line.find("Tower_Sniper:") == 0) currentType = TowerType::Sniper;
      else if (line.find("Tower_Multishot:") == 0) currentType = TowerType::Multishot;
      currentCategory = "";
      currentLevelIndex = -1;
      continue;
    }

    if (currentType != TowerType::Empty) {
      size_t colonPos = line.find(':');
      if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        size_t kEnd = key.find_last_not_of(" \t");
        if (kEnd != std::string::npos) key = key.substr(0, kEnd + 1);

        std::string valStr = line.substr(colonPos + 1);
        size_t vStart = valStr.find_first_not_of(" \t");
        if (vStart != std::string::npos) valStr = valStr.substr(vStart);
        size_t vEnd = valStr.find_last_not_of(" \t\r\n");
        if (vEnd != std::string::npos) valStr = valStr.substr(0, vEnd + 1);

        if (indent == 2) {
            currentCategory = "";
            currentLevelIndex = -1;
            if (!valStr.empty()) {
                float val = std::stof(valStr);
                if (key == "cost") towerConfigs[currentType].cost = static_cast<int>(val);
                else if (key == "build_time") towerConfigs[currentType].buildTime = val;
                else if (key == "damage") towerConfigs[currentType].damage = static_cast<int>(val);
                else if (key == "cooldown") towerConfigs[currentType].cooldown = val;
                else if (key == "range") towerConfigs[currentType].range = val;
                else if (key == "targets_base") towerConfigs[currentType].baseTargets = static_cast<int>(val);
            } else {
                if (key == "upgrade_timed") currentCategory = "upgrade_timed";
            }
        } else if (indent == 4) {
            if (valStr.empty()) {
                if (key == "upgrade_damage") currentCategory = "upgrade_damage";
                else if (key == "upgrade_speed") currentCategory = "upgrade_speed";
                else if (key == "upgrade_targets") currentCategory = "upgrade_targets";
            } else if (currentCategory == "upgrade_timed") {
                float val = std::stof(valStr);
                if (key == "cost") towerConfigs[currentType].timedUpgrade.cost = static_cast<int>(val);
                else if (key == "duration") towerConfigs[currentType].timedUpgrade.duration = val;
                else if (key == "damage_multiplier") towerConfigs[currentType].timedUpgrade.damageMultiplier = val;
                else if (key == "cooldown_multiplier") towerConfigs[currentType].timedUpgrade.cooldownMultiplier = val;
            }
        } else if (indent == 6) {
            if (key.find("level_") == 0) {
                currentLevelIndex = std::stoi(key.substr(6)) - 1;
                if (currentCategory == "upgrade_damage") {
                    if (towerConfigs[currentType].damageUpgrades.size() <= static_cast<size_t>(currentLevelIndex))
                        towerConfigs[currentType].damageUpgrades.resize(currentLevelIndex + 1);
                } else if (currentCategory == "upgrade_speed") {
                    if (towerConfigs[currentType].speedUpgrades.size() <= static_cast<size_t>(currentLevelIndex))
                        towerConfigs[currentType].speedUpgrades.resize(currentLevelIndex + 1);
                } else if (currentCategory == "upgrade_targets") {
                    if (towerConfigs[currentType].targetUpgrades.size() <= static_cast<size_t>(currentLevelIndex))
                        towerConfigs[currentType].targetUpgrades.resize(currentLevelIndex + 1);
                }
            }
        } else if (indent == 8) {
            if (!valStr.empty() && currentLevelIndex >= 0) {
                float val = std::stof(valStr);
                if (currentCategory == "upgrade_damage") {
                    if (key == "cost") towerConfigs[currentType].damageUpgrades[currentLevelIndex].first = static_cast<int>(val);
                    else if (key == "amount") towerConfigs[currentType].damageUpgrades[currentLevelIndex].second = static_cast<int>(val);
                } else if (currentCategory == "upgrade_speed") {
                    if (key == "cost") towerConfigs[currentType].speedUpgrades[currentLevelIndex].first = static_cast<int>(val);
                    else if (key == "amount") towerConfigs[currentType].speedUpgrades[currentLevelIndex].second = val;
                } else if (currentCategory == "upgrade_targets") {
                    if (key == "cost") towerConfigs[currentType].targetUpgrades[currentLevelIndex].first = static_cast<int>(val);
                    else if (key == "amount") towerConfigs[currentType].targetUpgrades[currentLevelIndex].second = static_cast<int>(val);
                }
            }
        }
      }
    }
  }
}

} // namespace Config
