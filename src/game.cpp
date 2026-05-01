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
#include <ctime>
#include <random>
#include <chrono>
#include "third_party/json/single_include/nlohmann/json.hpp"  // JSON library – make sure it's included in your project

using json = nlohmann::json;

// ========== Difficulty constants (matching README) ==========
const int MAX_ROOMS_EASY   = 10;
const int MAX_ROOMS_NORMAL = 15;
const int MAX_ROOMS_HARD   = 20;

const float MONSTER_HP_MULTIPLIER[3] = {0.8f, 1.0f, 1.3f};
const int TRAP_DAMAGE_MIN[3] = {5, 15, 30};   // Easy, Normal, Hard
const int TRAP_DAMAGE_MAX[3] = {15, 30, 50};

// ========== Constructor & Destructor ==========
Game::Game()
    : seed(std::time(nullptr)),difficulty(1), totalRooms(MAX_ROOMS_NORMAL), currentRoomIndex(0),
      isRunning(false), playerWin(false), player(nullptr), mapGen(nullptr) {
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
        std::cin >> choice;

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
    clearScreen();
    std::cout << "\n=== DIFFICULTY ===\n";
    std::cout << "0. Easy   (10 rooms, weak monsters, low trap damage)\n";
    std::cout << "1. Normal (15 rooms, standard)\n";
    std::cout << "2. Hard   (20 rooms, tough monsters, high trap damage)\n";
    std::cout << "Choice: ";
    std::cin >> difficulty;
    if (difficulty < 0 || difficulty > 2) difficulty = 1;

    switch (difficulty) {
        case 0: totalRooms = MAX_ROOMS_EASY; break;
        case 1: totalRooms = MAX_ROOMS_NORMAL; break;
        case 2: totalRooms = MAX_ROOMS_HARD; break;
    }
    std::cout << "Difficulty set. Total rooms: " << totalRooms << "\n";
    pause();
}

void Game::initGame() {
    // Clean up previous game data (if any)
    delete player;
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();

    // Get player name
    std::string name;
    clearScreen();
    std::cout << "Enter your name: ";
    std::cin >> name;
    playerName = name;

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

    std::cout << "\nGame started! Good luck, " << playerName << "!\n";
    pause();
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

    // Set global variables based on current difficulty
    g_monsterHpMultiplier = MONSTER_HP_MULT[difficulty];
    g_trapDamageMin = TRAP_DAMAGE_MIN[difficulty];
    g_trapDamageMax = TRAP_DAMAGE_MAX[difficulty];
}

void Game::generateRooms() {
    // Ask MapGenerator to build the whole dungeon.
    // The method generateAll() returns a vector of Room pointers.
    mapGen->initMapGenerator(totalRooms, difficulty);
    rooms = mapGen->getGeneratedRooms();
}

// ========== Main Game Loop ==========
void Game::gameLoop() {
    while (isRunning && currentRoomIndex < totalRooms) {
        clearScreen();
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
    if (currentRoomIndex >= static_cast<int>(rooms.size())) return;

    Room* currentRoom = rooms[currentRoomIndex];
    currentRoomIndex++;

    RoomType type = currentRoom->getRoomType();

    switch (type) {
        case RoomType::NORMAL:
        case RoomType::BOSS: {
            std::vector<Monster*> monsters = currentRoom->getMonsters();
            BattleSystem battle;          // 改为局部对象，不是指针
            bool playerLost = false;
            bool playerFled = false;

            for (auto monster : monsters) {
                if (playerLost || playerFled) break;

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
                    battle.applyRewards();   // 逃跑也应用已获得的奖励
                    break;
                } else if (result == BattleResult::PLAYER_WIN) {
                    // 每击败一个怪物就立即应用奖励（防止多个怪物时奖励累计丢失）
                    battle.applyRewards();
                }
            }

            // 处理最终结果
            if (playerLost) {
                isRunning = false;
                playerWin = false;
                return;
            }
            if (playerFled) {
                // 已经处理过逃跑日志和奖励，直接返回
                return;
            }
            // 全部怪物胜利
            playerWin = true;
            // 不再重复应用奖励（已在循环内应用），只显示最终战报（可选）
            // 显示最后一次战斗的日志作为参考
            std::cout << battle.showBattleLog() << std::endl;
            break;
        }

        case RoomType::SHOP: {
            // 修复内存泄漏：使用局部对象，或者手动 delete
            Merchant* merchant = new Merchant(difficulty, seed);
            Shop shop;
            shop.initShop(merchant, player);
            shop.showShopUI();   // 假设需要显示商店界面，否则可以调用其他交互函数
            // 注意：Shop 不会自动删除 merchant，需要在 shop 内部管理或手动释放
            delete merchant;
            break;
        }

        case RoomType::TREASURE: {
            // 简单实现：给一点随机金币（可根据需要扩展）
            int gold = 50 + rand() % 151;  // 50~200 金币
            player->change_Money(gold);
            std::cout << "You found a treasure chest! Gained " << gold << " gold.\n";
            break;
        }

        default:
            break;
    }

    // Auto-save after each room
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

