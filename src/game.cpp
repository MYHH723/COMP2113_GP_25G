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
#include "third_party/json/single_include/nlohmann/json.hpp"  // JSON library - make sure it's included in your project

using json = nlohmann::json;

// ========== Difficulty constants (matching README) ==========
const int MAX_ROOMS_EASY   = 10;
const int MAX_ROOMS_NORMAL = 15;
const int MAX_ROOMS_HARD   = 20;

// ========== Constructor & Destructor ==========
Game::Game()
    : seed(std::time(nullptr)),difficulty(1), totalRooms(MAX_ROOMS_NORMAL), currentRoomIndex(0),
      isRunning(false), playerWin(false), pendingNewGameWelcome(false), player(nullptr), mapGen(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Game::~Game() {
    delete player;
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();
}

// ========== Menu and Initialization ==========
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

void Game::initGame() {
    // Clean up previous game data (if any)
    delete player;
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();

    // Get player name (line-based, same idea as battle choice: empty line re-prompts)
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

    // Create player (stats follow README: HP=1000, ATK=100, DEF=100, starting gold=500)
    player = new Player(playerName);

    // Create map generator
    std::mt19937 rng;
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);
    mapGen = new MapGenerator(seed);

    // Apply difficulty scaling (sets global modifiers for monsters/traps)
    applyDifficultyScaling();

    // Generate all rooms using the map generator
    generateRooms();

    currentRoomIndex = 0;
    isRunning = true;
    playerWin = false;
    pendingNewGameWelcome = true;
}

// ========== Global difficulty scaling variables ==========
float g_monsterHpMultiplier = 1.0f;
int g_trapDamageMin = 0;
int g_trapDamageMax = 0;

void Game::applyDifficultyScaling() {
    // Monster HP multipliers (Easy: 0.8, Normal: 1.0, Hard: 1.3)
    const float MONSTER_HP_MULT[] = {0.8f, 1.0f, 1.3f};

    // Trap damage ranges (Increased to be meaningful for 1000 HP)
    // Easy:    80 - 150
    // Normal: 150 - 280
    // Hard:   250 - 450
    const int TRAP_DAMAGE_MIN[] = {80, 150, 250};
    const int TRAP_DAMAGE_MAX[] = {150, 280, 450};

    // Set global variables based on current difficulty (clamp index for corrupt saves)
    const int di = std::max(0, std::min(2, difficulty));
    g_monsterHpMultiplier = MONSTER_HP_MULT[di];
    g_trapDamageMin = TRAP_DAMAGE_MIN[di];
    g_trapDamageMax = TRAP_DAMAGE_MAX[di];
}

void Game::generateRooms() {
    // Ask MapGenerator to build the whole dungeon.
    // generateMap() must be called before reading generated rooms.
    mapGen->initMapGenerator(totalRooms, difficulty);
    mapGen->generateMap();
    rooms = mapGen->getGeneratedRooms();
}

// ========== Main Game Loop ==========
void Game::gameLoop() {
    while (isRunning && currentRoomIndex < totalRooms) {
        clearScreen();
        if (pendingNewGameWelcome) {
            std::cout << "Game started! Good luck, " << playerName << "!\n\n";
            pendingNewGameWelcome = false;
        }
        std::cout << "\n===== Room " << (currentRoomIndex + 1) << " / " << totalRooms << " =====\n";

        // Show player status (using Player's display method)
        ConsoleUI::showPlayerStatus(*player);

        // Process the current room
        enterNextRoom();

        // Check if game should continue
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

void Game::enterNextRoom() {
    if (currentRoomIndex >= static_cast<int>(rooms.size())) {
        // Defensive guard: if rooms were not generated correctly,
        // stop the loop instead of repeatedly showing the same room index.
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

            // 用于累积整个房间的总奖励
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
                    // 逃跑时，已经获得的奖励（之前击败怪物的）也应该保留，但这里先显示日志并应用已累积部分
                    std::cout << battle.showBattleLog() << std::endl;
                    // 注意：逃跑时可能已经击败了一部分怪物，需要把已经累积的奖励给玩家
                    player->change_EXP(totalExp);
                    player->change_Money(totalGold);
                    player->change_score(totalScore);
                    // 再应用本场战斗（逃跑这场）的奖励（如果有，但逃跑时怪物未死，通常没有奖励）
                    battle.applyRewards();
                    saveGame();
                    return;
                } else if (result == BattleResult::PLAYER_WIN) {
                    // 累积本只怪物的奖励，但不立即应用
                    const float* rewards = battle.getRewards();
                    totalExp += rewards[0];
                    totalGold += rewards[1];
                    totalScore += rewards[2];
                }
            }

            // 处理最终结果
            if (playerLost) {
                // 死亡：不给予任何奖励（已经扣除血量等，游戏结束）
                isRunning = false;
                playerWin = false;
                return;
            }
            if (playerFled) {
                // 已在上面处理，直接返回
                return;
            }

            // 全部怪物胜利：一次性发放所有累积的奖励
            player->change_EXP(totalExp);
            player->change_Money(totalGold);
            player->change_score(totalScore);

            playerWin = false;
            // 显示最后一次战斗的日志（或者显示胜利信息）
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
// ========== Game End Logic ==========
void Game::checkGameOver() {
    if (player->get_HP() <= 0) {
        isRunning = false;
        playerWin = false;
    } else if (currentRoomIndex >= totalRooms) {
        playerWin = true;
        isRunning = false;
    }
}

void Game::showGameResult() {
    clearScreen();
    if (playerWin) {
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

