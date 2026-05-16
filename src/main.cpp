#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Zamek w SFML");
    sf::Texture castleTexture;
    if (!castleTexture.loadFromFile("textures/zamek.png")) {
        std::cerr << "Błąd: Nie udało się wczytać pliku zamek.png!" << std::endl;
    }
    sf::Sprite castleSprite;
    castleSprite.setTexture(castleTexture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Black);
        window.draw(castleSprite);
        window.display();
    }

    return 0;
}
