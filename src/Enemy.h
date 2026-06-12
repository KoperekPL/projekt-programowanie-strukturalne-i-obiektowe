#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "GameObject.h"

struct EnemyStats {
    int maxHp = 25;
    float speed = 150.f;
    int castleDamage = 5;
    int playerDamage = 1;
};

class Enemy : public GameObject {
protected:
    sf::Sprite shape;
    const sf::Texture* texD;
    const sf::Texture* texS;
    const sf::Texture* texU;
    int animFrame;
    float animTime;
    size_t currentTargetIndex;
    float speed;
    int hp;
    int maxHp;
    int castleDamage;
    int playerDamage;
    const std::vector<sf::Vector2f>* path;
    bool m_hasPassedPlayer;

public:
    float slowTimer = 0.f;

    Enemy(sf::Vector2f startPos, const EnemyStats& stats, const sf::Texture* tD, const sf::Texture* tS, const sf::Texture* tU, const std::vector<sf::Vector2f>* path);
    virtual ~Enemy() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) const override;
    sf::Vector2f getPosition() const override { return shape.getPosition(); }

    bool hasReachedEnd() const;
    void takeDamage(int damage);
    bool isDead() const;
    
    int getCastleDamage() const { return castleDamage; }
    int getPlayerDamage() const { return playerDamage; }
    void die() { hp = 0; }
    bool hasPassedPlayer() const { return m_hasPassedPlayer; }
    void setPassedPlayer(bool val) { m_hasPassedPlayer = val; }
};

#endif
