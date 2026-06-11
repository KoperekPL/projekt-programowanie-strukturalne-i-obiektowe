#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include "Tower.h"
#include "Enemy.h"

namespace Config {
    void loadTowerConfig(const std::string& filepath, std::map<TowerType, TowerStats>& towerConfigs);
    void loadEnemyConfig(const std::string& filepath, std::map<std::string, EnemyStats>& enemyConfigs);
}

#endif // CONFIG_H
