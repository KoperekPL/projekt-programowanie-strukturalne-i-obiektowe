#include "Game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

enum class MenuState {
    Main,
    NewGameName,
    NewGameMap,
    LoadGameWorld,
    LoadGameFile
};

struct MenuButton {
    sf::RectangleShape rect;
    sf::Text text;
    std::string value;
};

int main() {
    sf::RenderWindow menuWindow(sf::VideoMode(800, 600), "Tower Defense - Main Menu");
    
    sf::Font font;
    bool hasFont = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!hasFont) {
        std::cerr << "Warning: Could not load font from C:/Windows/Fonts/arial.ttf\n";
    }

    sf::Text titleText("TOWER DEFENSE", font, 40);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(400.f - titleText.getLocalBounds().width / 2.f, 50.f);

    sf::Text promptText("", font, 24);
    promptText.setFillColor(sf::Color::Yellow);

    std::string inputText = "";
    sf::Text inputTextDisplay("", font, 24);
    inputTextDisplay.setFillColor(sf::Color::White);

    MenuState state = MenuState::Main;
    
    // Main Menu Buttons
    sf::RectangleShape newGameBtn(sf::Vector2f(250.f, 60.f));
    newGameBtn.setFillColor(sf::Color(50, 150, 50));
    newGameBtn.setPosition(275.f, 250.f);
    sf::Text newGameText("NOWA GRA", font, 24);
    newGameText.setFillColor(sf::Color::White);
    newGameText.setPosition(400.f - newGameText.getLocalBounds().width / 2.f, 265.f);

    sf::RectangleShape loadGameBtn(sf::Vector2f(250.f, 60.f));
    loadGameBtn.setFillColor(sf::Color(50, 50, 150));
    loadGameBtn.setPosition(275.f, 350.f);
    sf::Text loadGameText("WCZYTAJ GRE", font, 24);
    loadGameText.setFillColor(sf::Color::White);
    loadGameText.setPosition(400.f - loadGameText.getLocalBounds().width / 2.f, 365.f);

    std::string chosenWorldName = "";
    std::string chosenMapPath = "";
    std::string chosenSaveFile = "";
    bool isNewGame = true;
    bool startGame = false;

    std::vector<MenuButton> dynamicBtns;

    auto createListButtons = [&](const std::vector<std::string>& items) {
        dynamicBtns.clear();
        float startY = 150.f;
        for (size_t i = 0; i < items.size(); ++i) {
            MenuButton mb;
            mb.rect.setSize(sf::Vector2f(400.f, 40.f));
            mb.rect.setFillColor(sf::Color(70, 70, 70));
            mb.rect.setPosition(200.f, startY + i * 50.f);
            
            mb.text.setFont(font);
            mb.text.setString(items[i]);
            mb.text.setCharacterSize(20);
            mb.text.setFillColor(sf::Color::White);
            mb.text.setPosition(210.f, startY + i * 50.f + 10.f);
            
            mb.value = items[i];
            dynamicBtns.push_back(mb);
        }
    };

    bool isFullscreen = false;

    while (menuWindow.isOpen()) {
        sf::Event event;
        while (menuWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                menuWindow.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
                isFullscreen = !isFullscreen;
                if (isFullscreen) {
                    menuWindow.create(sf::VideoMode::getDesktopMode(), "Tower Defense - Main Menu", sf::Style::Fullscreen);
                } else {
                    menuWindow.create(sf::VideoMode(800, 600), "Tower Defense - Main Menu", sf::Style::Default);
                }
                menuWindow.setView(sf::View(sf::FloatRect(0.f, 0.f, 800.f, 600.f)));
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                if (state != MenuState::Main) {
                    state = MenuState::Main;
                    inputText = "";
                }
            }
            if (event.type == sf::Event::TextEntered && state == MenuState::NewGameName) {
                if (event.text.unicode == '\b') {
                    if (!inputText.empty()) inputText.pop_back();
                } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!inputText.empty()) {
                        chosenWorldName = inputText;
                        state = MenuState::NewGameMap;
                        std::vector<std::string> maps;
                        if (fs::exists("../../../assets/map")) {
                            for (const auto& entry : fs::directory_iterator("../../../assets/map")) {
                                if (entry.path().extension() == ".map") {
                                    maps.push_back(entry.path().filename().string());
                                }
                            }
                        }
                        createListButtons(maps);
                    }
                } else if (event.text.unicode < 128 && event.text.unicode > 31) {
                    inputText += static_cast<char>(event.text.unicode);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = menuWindow.mapPixelToCoords(sf::Mouse::getPosition(menuWindow));
                    
                    if (state == MenuState::Main) {
                        if (newGameBtn.getGlobalBounds().contains(mousePos)) {
                            state = MenuState::NewGameName;
                            inputText = "";
                        }
                        if (loadGameBtn.getGlobalBounds().contains(mousePos)) {
                            state = MenuState::LoadGameWorld;
                            std::vector<std::string> worlds;
                            fs::create_directories("../../../saves"); // Ensure saves folder exists
                            for (const auto& entry : fs::directory_iterator("../../../saves")) {
                                if (entry.is_directory()) {
                                    worlds.push_back(entry.path().filename().string());
                                }
                            }
                            createListButtons(worlds);
                        }
                    } else if (state == MenuState::NewGameMap) {
                        for (auto& btn : dynamicBtns) {
                            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                                chosenMapPath = "../../../assets/map/" + btn.value;
                                isNewGame = true;
                                startGame = true;
                                menuWindow.close();
                                break;
                            }
                        }
                    } else if (state == MenuState::LoadGameWorld) {
                        for (auto& btn : dynamicBtns) {
                            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                                chosenWorldName = btn.value;
                                state = MenuState::LoadGameFile;
                                std::vector<std::string> saves;
                                std::string dir = "../../../saves/" + chosenWorldName;
                                if (fs::exists(dir)) {
                                    for (const auto& entry : fs::directory_iterator(dir)) {
                                        if (entry.path().extension() == ".save") {
                                            saves.push_back(entry.path().filename().string());
                                        }
                                    }
                                }
                                createListButtons(saves);
                                break;
                            }
                        }
                    } else if (state == MenuState::LoadGameFile) {
                        for (auto& btn : dynamicBtns) {
                            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                                chosenSaveFile = "../../../saves/" + chosenWorldName + "/" + btn.value;
                                isNewGame = false;
                                startGame = true;
                                menuWindow.close();
                                break;
                            }
                        }
                    }
                }
            }
        }

        menuWindow.clear(sf::Color(30, 30, 30));
        if (hasFont) menuWindow.draw(titleText);

        if (state == MenuState::Main) {
            menuWindow.draw(newGameBtn);
            menuWindow.draw(loadGameBtn);
            if (hasFont) {
                menuWindow.draw(newGameText);
                menuWindow.draw(loadGameText);
            }
        } else if (state == MenuState::NewGameName) {
            promptText.setString("Wprowadz nazwe swiata (i nacisnij ENTER):");
            promptText.setPosition(400.f - promptText.getLocalBounds().width / 2.f, 250.f);
            inputTextDisplay.setString(inputText + "_");
            inputTextDisplay.setPosition(400.f - inputTextDisplay.getLocalBounds().width / 2.f, 300.f);
            if (hasFont) {
                menuWindow.draw(promptText);
                menuWindow.draw(inputTextDisplay);
            }
        } else if (state == MenuState::NewGameMap) {
            promptText.setString("Wybierz mape:");
            promptText.setPosition(400.f - promptText.getLocalBounds().width / 2.f, 100.f);
            if (hasFont) menuWindow.draw(promptText);
            for (const auto& btn : dynamicBtns) {
                menuWindow.draw(btn.rect);
                if (hasFont) menuWindow.draw(btn.text);
            }
        } else if (state == MenuState::LoadGameWorld) {
            promptText.setString("Wybierz zapisany swiat:");
            promptText.setPosition(400.f - promptText.getLocalBounds().width / 2.f, 100.f);
            if (hasFont) menuWindow.draw(promptText);
            for (const auto& btn : dynamicBtns) {
                menuWindow.draw(btn.rect);
                if (hasFont) menuWindow.draw(btn.text);
            }
            if (dynamicBtns.empty()) {
                sf::Text emptyText("Brak zapisanych swiatow. Wcisnij ESC by wrocic.", font, 20);
                emptyText.setFillColor(sf::Color::Red);
                emptyText.setPosition(400.f - emptyText.getLocalBounds().width / 2.f, 200.f);
                if (hasFont) menuWindow.draw(emptyText);
            }
        } else if (state == MenuState::LoadGameFile) {
            promptText.setString("Wybierz plik zapisu dla: " + chosenWorldName);
            promptText.setPosition(400.f - promptText.getLocalBounds().width / 2.f, 100.f);
            if (hasFont) menuWindow.draw(promptText);
            for (const auto& btn : dynamicBtns) {
                menuWindow.draw(btn.rect);
                if (hasFont) menuWindow.draw(btn.text);
            }
        }

        menuWindow.display();
    }

    if (startGame) {
        if (isNewGame) {
            std::string savePath = "../../../saves/" + chosenWorldName + "/save.save";
            Game game(savePath, chosenMapPath, true);
            game.run();
        } else {
            Game game(chosenSaveFile, "", false);
            game.run();
        }
    }

    return 0;
}
