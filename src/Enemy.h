#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <vector>

class Enemy {
protected:
    sf::Sprite shape;
    const sf::Texture* texD;
    const sf::Texture* texS;
    const sf::Texture* texU;
    int animFrame;
    float animTime;
    size_t currentTargetIndex;
    float speed;

public:
    Enemy(sf::Vector2f startPos, const sf::Texture* tD, const sf::Texture* tS, const sf::Texture* tU);
    virtual ~Enemy() = default;

    virtual void update(float dt, const std::vector<sf::Vector2f>& pathPoints);
    virtual void draw(sf::RenderWindow& window) const;

    bool hasReachedEnd(size_t pathSize) const;
    sf::Vector2f getPosition() const { return shape.getPosition(); }
};

#endif
