#ifndef TOWER_H
#define TOWER_H
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class TowerType {
    Empty,
    Base,
    Sniper,
    Multishot
};

enum class TowerState {
    Idle,
    Building,
    Upgrading
};

struct TimedUpgradeStats {
    int cost = 0;
    float duration = 0.f;
    float damageMultiplier = 1.f;
    float cooldownMultiplier = 1.f;
};

struct TowerStats {
    int cost = 0;
    float buildTime = 0.f;
    int damage = 0;
    float cooldown = 0.f;
    float range = 0.f;

    std::vector<std::pair<int, int>> damageUpgrades;
    std::vector<std::pair<int, float>> speedUpgrades;
    
    // Dla Multishot
    int baseTargets = 1;
    // <Koszt, Ilosc celow>
    std::vector<std::pair<int, int>> targetUpgrades;

    TimedUpgradeStats timedUpgrade;
};

struct Tower {
    sf::Vector2f position;
    TowerType type = TowerType::Empty;
    TowerState state = TowerState::Idle;
    float buildTimer = 0.f;
    float cooldownTimer = 0.f;
    
    int damageUpgradeLevel = 0;
    int speedUpgradeLevel = 0;
    int targetsUpgradeLevel = 0;

    bool hasTimedUpgrade = false;
    float timedUpgradeTimer = 0.f;
};

#endif
