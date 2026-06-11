#include "Enemy.h"
#include <cmath>

Enemy::Enemy(sf::Vector2f startPos, const EnemyStats& stats, const sf::Texture* tD, const sf::Texture* tS, const sf::Texture* tU) : currentTargetIndex(1), speed(stats.speed), texD(tD), texS(tS), texU(tU), animFrame(0), animTime(0.f), hp(stats.maxHp), maxHp(stats.maxHp), castleDamage(stats.castleDamage), playerDamage(stats.playerDamage) {
    if (texD) {
        shape.setTexture(*texD);
        shape.setTextureRect(sf::IntRect(0, 0, 48, 48));
        shape.setOrigin(24.f, 24.f);
    }
    shape.setPosition(startPos);
}

void Enemy::update(float dt, const std::vector<sf::Vector2f>& pathPoints) {
    if (currentTargetIndex < pathPoints.size()) {
        sf::Vector2f target = pathPoints[currentTargetIndex];
        sf::Vector2f current = shape.getPosition();
        sf::Vector2f dir = target - current;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        
        if (dist > 0.f) {
            animTime += dt;
            if (animTime >= 0.1f) {
                animTime = 0.f;
                animFrame = (animFrame + 1) % 6;
                
                if (std::abs(dir.x) > std::abs(dir.y)) {
                    if (texS) shape.setTexture(*texS);
                    if (dir.x < 0) {
                        shape.setScale(-1.f, 1.f);
                    } else {
                        shape.setScale(1.f, 1.f);
                    }
                } else {
                    shape.setScale(1.f, 1.f);
                    if (dir.y < 0) {
                        if (texU) shape.setTexture(*texU);
                    } else {
                        if (texD) shape.setTexture(*texD);
                    }
                }
                shape.setTextureRect(sf::IntRect(animFrame * 48, 0, 48, 48));
            }
        }

        if (dist <= speed * dt) {
            shape.setPosition(target);
            currentTargetIndex++;
        } else {
            dir /= dist;
            shape.move(dir * speed * dt);
        }
    }
}

void Enemy::draw(sf::RenderWindow& window) const {
    window.draw(shape);
    if (hp < maxHp) {
        sf::RectangleShape bg(sf::Vector2f(40.f, 5.f));
        bg.setFillColor(sf::Color::Red);
        bg.setOrigin(20.f, 2.5f);
        bg.setPosition(shape.getPosition().x, shape.getPosition().y - 30.f);
        
        sf::RectangleShape fg(sf::Vector2f(40.f * ((float)hp / maxHp), 5.f));
        fg.setFillColor(sf::Color::Green);
        fg.setOrigin(20.f, 2.5f);
        fg.setPosition(shape.getPosition().x, shape.getPosition().y - 30.f);
        
        window.draw(bg);
        window.draw(fg);
    }
}

void Enemy::takeDamage(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}

bool Enemy::isDead() const {
    return hp <= 0;
}

bool Enemy::hasReachedEnd(size_t pathSize) const {
    return currentTargetIndex >= pathSize;
}
