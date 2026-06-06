// game.cpp
// Implementation of the Game class.

#include "game.h"
#include "player.h"
#include "mapgenerator.h"
#include "room.h"
#include "monster.h"
#include "battlesystem.h"
#include "shop.h"
#include "utils.h"
#include "merchant.h"
#include "consoleUI.h"
#include "trap.h"
#include "balance.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <memory>

// ========== Difficulty constants (matching README) ==========
const int MAX_ROOMS_EASY   = 10;
const int MAX_ROOMS_NORMAL = 15;
const int MAX_ROOMS_HARD   = 20;

namespace {

void printRandomFlavor(const char* const* lines, size_t count) {
    if (count == 0) return;
    const size_t idx = static_cast<size_t>(getRandom(0, static_cast<int>(count) - 1));
    std::cout << "  * " << lines[idx] << "\n";
}

const char* const kNewGameWelcome[] = {
    "Game started! Good luck, ",
    "The vaults await, ",
    "By torch and steel, ",
    "Fortune favour you, ",
    "May your mail hold and your purse grow, ",
};

const char* const kGoblinNestIntro[] = {
    "A stench of rot and squealing - goblins pour from every crevice!",
    "Yellow eyes blink in the dark; the nest awakens hungry.",
    "Three goblins scramble forth, chittering like devils at compline.",
    "The nest boils over; small foes, savage teeth.",
    "Goblins! A writhing pack spills across the stones toward you.",
};

const char* const kAnotherFoe[] = {
    "Ready your steel - the hall is not yet quiet.",
    "Another shape lurches from the gloom, unwilling to yield.",
    "The first lies broken; a second hungers for the tale.",
    "Steel still warm, yet another foe demands audience.",
    "The chamber is not satisfied - one more stands defiant.",
};

const char* const kRetreatAfterFlee[] = {
    "You retreated. This room remains ahead of you.",
    "The hall keeps its secrets; you live to try again.",
    "Withdrawal is not defeat - the stones will wait.",
    "You escape with breath intact; the foe keeps the ground.",
    "Live today; the dungeon is patient, and so must you be.",
};

const char* const kShopIntro[] = {
    "A pedlar's stall glows beneath sputtering torchlight - the merchant has set up camp.",
    "The clink of coin and scent of oiled steel mark a trader's haven in the deep.",
    "Wares hang from rusted hooks; a smiling merchant bids you spend your gold.",
    "Shelter from the slaughter, for a price - the travelling seller awaits.",
    "Canvas, counter, and coffers - fortune sells its wares in this hall.",
};

const char* const kTreasureIntro[] = {
    "Gold glints from a chest, but the chamber may not be empty...",
    "A coffer gleams beneath cobwebs - silence is seldom honest here.",
    "Treasure hoarded by dead hands; perhaps guarded by living ones.",
    "Coins wink like false stars; tread softly, pilgrim.",
    "Riches and ruin often share one lock - proceed with steel ready.",
};

const char* const kChestOpen[] = {
    "You open the chest! Gained ",
    "The lock surrenders; you claim ",
    "Dead men's gold becomes yours: ",
    "The hoard yields ",
    "With a rusted groan, the chest releases ",
};

const char* const kRoomCleared[] = {
    "Room cleared!",
    "The hall falls silent - for now, you stand victorious.",
    "Foes broken; the stones grant you brief peace.",
    "Another chamber conquered; your legend grows by a line.",
    "Silence returns, heavy as a chapel after battle.",
};

const char* const kTrapAftermath[] = {
    "The mechanism groans and stills, sated with mischief.",
    "Dust settles; pain remains - press on or perish.",
    "Ancient malice spent itself in one cruel moment.",
    "The trap sleeps again, content with its toll.",
    "Stone and spring remember old architects of sorrow.",
};

void runPreRoomMenu(Player* player, int roomNumber, int totalRooms,
                    const std::string& welcomeBanner) {
    if (!player) return;
    ConsoleUI::runPreRoomMenu(*player, roomNumber, totalRooms, welcomeBanner);
}

} // namespace

// ========== Constructor & Destructor ==========
Game::Game()
    : seed(std::time(nullptr)), difficulty(1), totalRooms(MAX_ROOMS_NORMAL), currentRoomIndex(0),
      isRunning(false), playerWin(false), pendingNewGameWelcome(false) {
}

Game::~Game() = default;

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
            if (!selectDifficulty()) {
                continue;
            }
            initGame();
            gameLoop();
        } else if (choice == 2) {
            const LoadResult lr = loadGame();
            if (lr == LoadResult::Loaded) {
                gameLoop();
            } else if (lr == LoadResult::Failed) {
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

bool Game::selectDifficulty() {
    int d = 0;
    while (true) {
        clearScreen();
        std::cout << "\n=== DIFFICULTY ===\n";
        std::cout << "1. Easy   (10 rooms, weak monsters, low trap damage)\n";
        std::cout << "2. Normal (15 rooms, standard)\n";
        std::cout << "3. Hard   (20 rooms, tough monsters, high trap damage)\n";
        std::cout << "0. Back to main menu\n";
        std::cout << "Choice (0-3): ";

        if (!(std::cin >> d)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "\nInvalid input. Enter a single whole number: 1, 2, or 3.\n";
            pause();
            continue;
        }
        discardRestOfLine();

        if (d == 0) {
            return false;
        }
        if (d < 1 || d > 3) {
            std::cout << "\nInvalid choice. Enter 0 to go back, or 1, 2, or 3.\n";
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
        return true;
    }
}

void Game::initGame() {
    // Clean up previous game data (if any)
    player.reset();
    mapGen.reset();
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

    // Create player (stats: see player.h DEFAULT_* / README Player Stats)
    player.reset(new Player(playerName));

    seed = static_cast<int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    seedGameRandom(static_cast<unsigned>(seed));
    mapGen.reset(new MapGenerator(seed));

    // Apply difficulty scaling (sets global modifiers for monsters/traps)
    applyDifficultyScaling();

    // Generate all rooms using the map generator
    generateRooms();

    currentRoomIndex = 0;
    isRunning = true;
    playerWin = false;
    pendingNewGameWelcome = true;
}

void Game::applyDifficultyScaling() {
    // Monster HP multipliers (Easy: 0.8, Normal: 1.0, Hard: 1.3)
    const float MONSTER_HP_MULT[] = {0.8f, 1.0f, 1.3f};

    // Trap damage ranges (scaled for DEFAULT_HP = 100; per-type multiplier in trap.cpp)
    // Easy: 5-12, Normal: 8-18, Hard: 12-25
    const int TRAP_DAMAGE_MIN[] = {5, 8, 12};
    const int TRAP_DAMAGE_MAX[] = {12, 18, 25};

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
    rooms = mapGen->releaseRooms();
}

bool Game::triggerRoomTraps(Room* room) {
    if (!room || !player) return false;

    const auto& traps = room->getTraps();
    for (const auto& trapPtr : traps) {
        Trap* trap = trapPtr.get();
        if (!trap || !trap->get_isActive()) continue;

        std::cout << "\n*** TRAP! ***\n" << trap->getDescription() << "\n";
        const float hpBefore = player->get_HP();
        trap->triggerTrap(*player);
        const int damage = static_cast<int>(hpBefore - player->get_HP());
        std::cout << "You take " << damage << " damage! HP: "
                  << static_cast<int>(player->get_HP()) << "\n";
        printRandomFlavor(kTrapAftermath, sizeof(kTrapAftermath) / sizeof(kTrapAftermath[0]));
        pause();

        if (!player->get_isAlive()) {
            isRunning = false;
            playerWin = false;
            return true;
        }
    }
    return false;
}

Game::RoomCombatResult Game::fightRoomMonsters(Room* room) {
    if (!room || !player) return RoomCombatResult::Empty;

    const auto& monsters = room->getMonsters();
    if (monsters.empty()) return RoomCombatResult::Empty;

    BattleSystem battle;
    float totalExp = 0.0f;
    float totalGold = 0.0f;
    float totalScore = 0.0f;

    const size_t monsterCount = monsters.size();
    for (size_t mi = 0; mi < monsterCount; ++mi) {
        Monster* monster = monsters[mi].get();
        if (!monster || !monster->get_isAlive()) continue;

        if (mi > 0) {
            std::cout << "\n========================================\n";
            std::cout << "  Another foe appears! ("
                      << (mi + 1) << " / " << monsterCount << ")\n";
            printRandomFlavor(kAnotherFoe, sizeof(kAnotherFoe) / sizeof(kAnotherFoe[0]));
            std::cout << "========================================\n";
            pause();
        }

        battle.initBattle(player.get(), monster);
        battle.startBattle();

        while (battle.get_isBattleActive()) {
            battle.executeBattleRound();
        }
        battle.endBattle();

        const BattleResult result = battle.getLastResult();
        if (result == BattleResult::PLAYER_LOSE) {
            isRunning = false;
            playerWin = false;
            return RoomCombatResult::Lost;
        }
        if (result == BattleResult::PLAYER_FLEE) {
            const float* rewards = battle.getRewards();
            totalExp += rewards[0];
            totalGold += rewards[1];
            totalScore += rewards[2];
            player->change_EXP(totalExp);
            player->change_Money(totalGold);
            player->change_score(totalScore);
            std::cout << battle.showBattleLog() << std::endl;
            return RoomCombatResult::Fled;
        }
        if (result == BattleResult::PLAYER_WIN) {
            const float* rewards = battle.getRewards();
            totalExp += rewards[0];
            totalGold += rewards[1];
            totalScore += rewards[2];
        }
    }

    player->change_EXP(totalExp);
    player->change_Money(totalGold);
    player->change_score(totalScore);
    room->clearRoom();

    printRandomFlavor(kRoomCleared, sizeof(kRoomCleared) / sizeof(kRoomCleared[0]));
    std::cout << " Gained " << formatFixed2(totalExp) << " EXP, "
              << formatFixed2(totalGold) << " Gold, " << formatFixed2(totalScore) << " Score.\n";
    std::cout << battle.showBattleLog() << std::endl;
    return RoomCombatResult::Cleared;
}

// ========== Main Game Loop ==========
void Game::gameLoop() {
    while (isRunning && currentRoomIndex < totalRooms) {
        std::string welcomeBanner;
        if (pendingNewGameWelcome) {
            const size_t wi = static_cast<size_t>(
                getRandom(0, static_cast<int>(sizeof(kNewGameWelcome) / sizeof(kNewGameWelcome[0])) - 1));
            welcomeBanner = std::string(kNewGameWelcome[wi]) + playerName + "!\n\n";
            pendingNewGameWelcome = false;
        }
        runPreRoomMenu(player.get(), currentRoomIndex + 1, totalRooms, welcomeBanner);

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

    Room* currentRoom = rooms[static_cast<size_t>(currentRoomIndex)].get();
    bool advanceRoom = false;
    bool verboseSave = false;

    RoomType type = currentRoom->getRoomType();

    switch (type) {
        case RoomType::NORMAL:
        case RoomType::BOSS: {
            if (triggerRoomTraps(currentRoom)) {
                saveGame();
                return;
            }

            if (currentRoom->isGoblinNestRoom()) {
                std::cout << "\n*** GOBLIN NEST ***\n";
                printRandomFlavor(kGoblinNestIntro,
                                  sizeof(kGoblinNestIntro) / sizeof(kGoblinNestIntro[0]));
                std::cout << "\n";
                pause();
            }

            const RoomCombatResult combat = fightRoomMonsters(currentRoom);
            if (combat == RoomCombatResult::Lost) return;
            if (combat == RoomCombatResult::Fled) {
                printRandomFlavor(kRetreatAfterFlee,
                                  sizeof(kRetreatAfterFlee) / sizeof(kRetreatAfterFlee[0]));
                saveGame();
                return;
            }
            advanceRoom = true;
            break;
        }

        case RoomType::SHOP: {
            std::cout << "\n*** MERCHANT SHOP ***\n";
            printRandomFlavor(kShopIntro, sizeof(kShopIntro) / sizeof(kShopIntro[0]));
            std::cout << "\n";
            pause();
            Merchant merchant(difficulty, seed);
            Shop shop;
            shop.initShop(&merchant, player.get());
            shop.showShopUI();
            advanceRoom = true;
            verboseSave = true;
            break;
        }

        case RoomType::TREASURE: {
            std::cout << "\n*** TREASURE ROOM ***\n";
            printRandomFlavor(kTreasureIntro, sizeof(kTreasureIntro) / sizeof(kTreasureIntro[0]));
            std::cout << "\n";

            if (triggerRoomTraps(currentRoom)) {
                saveGame();
                return;
            }

            const RoomCombatResult combat = fightRoomMonsters(currentRoom);
            if (combat == RoomCombatResult::Lost) return;
            if (combat == RoomCombatResult::Fled) {
                printRandomFlavor(kRetreatAfterFlee,
                                  sizeof(kRetreatAfterFlee) / sizeof(kRetreatAfterFlee[0]));
                saveGame();
                return;
            }

            const int gold = getRandom(50, 200);
            player->change_Money(static_cast<float>(gold));
            currentRoom->clearRoom();
            const size_t ci = static_cast<size_t>(
                getRandom(0, static_cast<int>(sizeof(kChestOpen) / sizeof(kChestOpen[0])) - 1));
            std::cout << kChestOpen[ci] << gold << " gold.\n";
            advanceRoom = true;
            break;
        }

        default:
            advanceRoom = true;
            break;
    }

    if (advanceRoom) {
        currentRoomIndex++;
    }
    saveGame(verboseSave);
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
        std::cout << "You died in room " << (currentRoomIndex + 1) << ".\n";
    }
    ConsoleUI::showPlayerStatus(*player);
    pause();
}

