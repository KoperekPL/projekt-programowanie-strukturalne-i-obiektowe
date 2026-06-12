#include "Tower.h"

void Tower::update(float dt) {
    if (state == TowerState::Building || state == TowerState::Upgrading) {
        buildTimer -= dt;
        if (buildTimer <= 0.f) {
            state = TowerState::Idle;
            buildTimer = 0.f;
            buildSound.stop();
        }
    } else if (state == TowerState::Idle) {
        if (cooldownTimer > 0.f) {
            cooldownTimer -= dt;
        }
        if (hasTimedUpgrade) {
            timedUpgradeTimer -= dt;
            if (timedUpgradeTimer <= 0.f) {
                hasTimedUpgrade = false;
            }
        }
    }

    // Rotation logic
    float diff = targetRotation - currentRotation;
    while (diff <= -180.f) diff += 360.f;
    while (diff > 180.f) diff -= 360.f;

    if (diff > 0) {
        currentRotation += rotationSpeed * dt;
        if (currentRotation > targetRotation) currentRotation = targetRotation;
    } else if (diff < 0) {
        currentRotation -= rotationSpeed * dt;
        if (currentRotation < targetRotation) currentRotation = targetRotation;
    }
}

void Tower::draw(sf::RenderWindow& window) const {
    // Drawing is handled by Game.cpp for now, but we can move it here later.
}
