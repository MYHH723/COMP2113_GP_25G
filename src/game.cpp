#include "savegame.h"  
// game.cpp
// Implementation of the Game class.

#include "game.h"
#include "player.h"
#include "mapgenerator.h"
#include "room.h"
#include "battlesystem.h"
#include "shop.h"
#include "utils.h"
#include "merchant.h"
#include "consoleUI.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <ctime>
#include <random>
#include <chrono>
#include <limits>
#include "third_party/json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

// Difficulty settings for total rooms per difficulty level
const int MAX_ROOMS_EASY   = 10;
const int MAX_ROOMS_NORMAL = 15;
const int MAX_ROOMS_HARD   = 20;

// Game constructor: Initialize default game state
Game::Game()
    : seed(std::time(nullptr)),difficulty(1), totalRooms(MAX_ROOMS_NORMAL), currentRoomIndex(0),
      isRunning(false), playerWin(false), pendingNewGameWelcome(false), player(nullptr), mapGen(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

// Game destructor: Clean up dynamically allocated memory
Game::~Game() {
    delete player;
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();
}

// Display main menu and handle user selection
void Game::showMainMenu() {
    int choice = 0;
    while (true) {
        clearScreen();
        std::cout << "\n================================\n";
        std::cout << "     ROGUELIKE DUNGEON GAME\n";
        std::cout << "================================\n";
        std::cout << "1. New Game\n";
        std::cout << "2. Load Game\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "Invalid input. Enter a number (1-3).\n";
            pause();
            continue;
        }
        discardRestOfLine();

        if (choice == 1) {
            selectDifficulty();
            initGame();
            gameLoop();
        } else if (choice == 2) {
            loadGame();
            if (isRunning) {
                gameLoop();
            } else {
                std::cout << "Load failed or no save file.\n";
                pause();
            }
        } else if (choice == 3) {
            std::cout << "Goodbye!\n";
            break;
        } else {
            std::cout << "Invalid choice.\n";
            pause();
        }
    }
}

// Let player select game difficulty and set total rooms
void Game::selectDifficulty() {
    int d = 0;
    while (true) {
        clearScreen();
        std::cout << "\n=== DIFFICULTY ===\n";
        std::cout << "1. Easy   (10 rooms, weak monsters, low trap damage)\n";
        std::cout << "2. Normal (15 rooms, standard)\n";
        std::cout << "3. Hard   (20 rooms, tough monsters, high trap damage)\n";
        std::cout << "Choice (1-3): ";

        if (!(std::cin >> d)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "\nInvalid input. Enter a single whole number: 1, 2, or 3.\n";
            pause();
            continue;
        }
        discardRestOfLine();

        if (d < 1 || d > 3) {
            std::cout << "\nInvalid choice. You must enter 1, 2, or 3.\n";
            pause();
            continue;
        }

        difficulty = d - 1;
        switch (difficulty) {
            case 0: totalRooms = MAX_ROOMS_EASY; break;
            case 1: totalRooms = MAX_ROOMS_NORMAL; break;
            case 2: totalRooms = MAX_ROOMS_HARD; break;
        }
        std::cout << "\nDifficulty set. Total rooms: " << totalRooms << "\n";
        pause();
        return;
    }
}

// Initialize new game: player, map, rooms, and difficulty
void Game::initGame() {
    delete player;
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();

    clearScreen();
    discardRestOfLineIfBuffered();
    while (true) {
        std::cout << "Enter your name: " << std::flush;
        std::string name;
        if (!std::getline(std::cin, name)) {
            std::cin.clear();
            playerName = "Hero";
            break;
        }
        const size_t first = name.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            std::cout << "Enter a non-empty name.\n" << std::flush;
            continue;
        }
        const size_t last = name.find_last_not_of(" \t\r\n");
        playerName = name.substr(first, last - first + 1);
        break;
    }

    player = new Player(playerName);

    std::mt19937 rng;
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);
    mapGen = new MapGenerator(seed);

    applyDifficultyScaling();
    generateRooms();

    currentRoomIndex = 0;
    isRunning = true;
    playerWin = false;
    pendingNewGameWelcome = true;
}

// Global difficulty modifiers
float g_monsterHpMultiplier = 1.0f;
int g_trapDamageMin = 0;
int g_trapDamageMax = 0;

// Apply global scaling based on selected difficulty
void Game::applyDifficultyScaling() {
    const float MONSTER_HP_MULT[] = {0.8f, 1.0f, 1.3f};
    const int TRAP_DAMAGE_MIN[] = {80, 150, 250};
    const int TRAP_DAMAGE_MAX[] = {150, 280, 450};

    const int di = std::max(0, std::min(2, difficulty));
    g_monsterHpMultiplier = MONSTER_HP_MULT[di];
    g_trapDamageMin = TRAP_DAMAGE_MIN[di];
    g_trapDamageMax = TRAP_DAMAGE_MAX[di];
}

// Generate dungeon rooms using map generator
void Game::generateRooms() {
    mapGen->initMapGenerator(totalRooms, difficulty);
    mapGen->generateMap();
    rooms = mapGen->getGeneratedRooms();
}

// Main game loop: process rooms until game ends
void Game::gameLoop() {
    while (isRunning && currentRoomIndex < totalRooms) {
        clearScreen();
        if (pendingNewGameWelcome) {
            std::cout << "Game started! Good luck, " << playerName << "!\n\n";
            pendingNewGameWelcome = false;
        }
        std::cout << "\n===== Room " << (currentRoomIndex + 1) << " / " << totalRooms << " =====\n";

        ConsoleUI::showPlayerStatus(*player);
        enterNextRoom();

        if (!isRunning) break;
        if (currentRoomIndex >= totalRooms) {
            playerWin = true;
            break;
        }
        pause();
    }
    checkGameOver();
    showGameResult();
}

// Process room events: battle, shop, treasure
void Game::enterNextRoom() {
    if (currentRoomIndex >= static_cast<int>(rooms.size())) {
        isRunning = false;
        return;
    }

    Room* currentRoom = rooms[currentRoomIndex];
    currentRoomIndex++;
    RoomType type = currentRoom->getRoomType();

    switch (type) {
        case RoomType::NORMAL:
        case RoomType::BOSS: {
            std::vector<Monster*> monsters = currentRoom->getMonsters();
            BattleSystem battle;
            bool playerLost = false;
            bool playerFled = false;

            float totalExp = 0.0f;
            float totalGold = 0.0f;
            float totalScore = 0.0f;

            const size_t monsterCount = monsters.size();
            for (size_t mi = 0; mi < monsterCount; ++mi) {
                Monster* monster = monsters[mi];
                if (!monster) continue;
                if (playerLost || playerFled) break;

                if (mi > 0) {
                    std::cout << "\n========================================\n";
                    std::cout << "  Another foe appears! ("
                              << (mi + 1) << " / " << monsterCount << ")\n";
                    std::cout << "  Ready your steel - the hall is not yet quiet.\n";
                    std::cout << "========================================\n";
                    pause();
                }

                battle.initBattle(player, monster);
                battle.startBattle();

                while (battle.get_isBattleActive()) {
                    battle.executeBattleRound();
                }
                battle.endBattle();

                BattleResult result = battle.getLastResult();
                if (result == BattleResult::PLAYER_LOSE) {
                    playerLost = true;
                    break;
                } else if (result == BattleResult::PLAYER_FLEE) {
                    playerFled = true;
                    std::cout << battle.showBattleLog() << std::endl;
                    player->change_EXP(totalExp);
                    player->change_Money(totalGold);
                    player->change_score(totalScore);
                    battle.applyRewards();
                    saveGame();
                    return;
                } else if (result == BattleResult::PLAYER_WIN) {
                    const float* rewards = battle.getRewards();
                    totalExp += rewards[0];
                    totalGold += rewards[1];
                    totalScore += rewards[2];
                }
            }

            if (playerLost) {
                isRunning = false;
                playerWin = false;
                return;
            }
            if (playerFled) {
                return;
            }

            player->change_EXP(totalExp);
            player->change_Money(totalGold);
            player->change_score(totalScore);

            playerWin = false;
            std::cout << "Room cleared! Gained " << formatFixed2(totalExp) << " EXP, "
                      << formatFixed2(totalGold) << " Gold, " << formatFixed2(totalScore) << " Score.\n";
            std::cout << battle.showBattleLog() << std::endl;
            break;
        }

        case RoomType::SHOP: {
            Merchant* merchant = new Merchant(difficulty, seed);
            Shop shop;
            shop.initShop(merchant, player);
            shop.showShopUI();
            delete merchant;
            break;
        }

        case RoomType::TREASURE: {
            int gold = 50 + rand() % 151;
            player->change_Money(gold);
            std::cout << "You found a treasure chest! Gained " << gold << " gold.\n";
            break;
        }

        default:
            break;
    }

    saveGame();
}

// Check win/lose conditions
void Game::checkGameOver() {
    if (player->get_HP() <= 0) {
        isRunning = false;
        playerWin = false;
    } else if (currentRoomIndex >= totalRooms) {
        playerWin = true;
        isRunning = false;
    }
}

// Show final game result (victory or defeat)
void Game::showGameResult() {
    clearScreen();
    if (playerWin) {
        printVictoryArt(difficulty);
        std::cout << "\n================================\n";
        std::cout << "         YOU WIN!\n";
        std::cout << "================================\n";
        std::cout << "Congratulations, " << playerName << "!\n";
        std::cout << "You cleared all " << totalRooms << " rooms.\n";
    } else {
        std::cout << "\n================================\n";
        std::cout << "         GAME OVER\n";
        std::cout << "================================\n";
        std::cout << "You died in room " << currentRoomIndex << ".\n";
    }
    ConsoleUI::showPlayerStatus(*player);
    pause();
}
