#include <iostream>
#include <memory>
#include <sstream>
#include "room.h"
#include "monster.h"
#include "trap.h"
#include "player.h"
#include "utils.h"

namespace {

enum class EncounterKind { GOBLIN, SKELETON, GOBLIN_NEST };

// Per-room monster species roll (goblin nest only on Hard NORMAL rooms).
EncounterKind rollEncounterKind(int diff, RoomType type) {
    if (type == BOSS) {
        return EncounterKind::SKELETON;
    }
    const int roll = getRandom(0, 99);
    switch (diff) {
    case 0: // Easy: mostly goblins, rare skeletons
        return (roll < 12) ? EncounterKind::SKELETON : EncounterKind::GOBLIN;
    case 1: // Normal: more skeletons than Easy, still goblin-heavy
        return (roll < 22) ? EncounterKind::SKELETON : EncounterKind::GOBLIN;
    default: // Hard
        if (type == NORMAL && roll < 25) {
            return EncounterKind::GOBLIN_NEST;
        }
        return EncounterKind::SKELETON;
    }
}

int goblinLevel(int diff) {
    return (diff <= 0) ? 1 : 2;
}

int skeletonLevel(int diff) {
    return (diff <= 1) ? 3 : 4;
}

void spawnMonster(Monster* monster, int diff, EncounterKind kind) {
    switch (kind) {
    case EncounterKind::GOBLIN:
    case EncounterKind::GOBLIN_NEST:
        monster->initMonster(diff, goblinLevel(diff));
        break;
    case EncounterKind::SKELETON:
        monster->initMonster(diff, skeletonLevel(diff));
        break;
    }
}

// Chance that a combat room spawns any traps; if yes, roll count in [minTraps, maxTraps].
int rollTrapCount(int diff, RoomType type) {
    int spawnChance = 0;
    int minTraps = 0;
    int maxTraps = 0;

    switch (type) {
    case NORMAL:
        switch (diff) {
        case 0: spawnChance = 35; minTraps = 1; maxTraps = 1; break; // Easy: 35%
        case 1: spawnChance = 45; minTraps = 1; maxTraps = 2; break; // Normal: 45%
        default: spawnChance = 55; minTraps = 1; maxTraps = 3; break; // Hard: 55%
        }
        break;
    case BOSS:
        switch (diff) {
        case 0: spawnChance = 50; minTraps = 1; maxTraps = 2; break;
        case 1: spawnChance = 60; minTraps = 1; maxTraps = 2; break;
        default: spawnChance = 70; minTraps = 2; maxTraps = 3; break;
        }
        break;
    case TREASURE:
        return (getRandom(0, 99) < 35) ? 1 : 0;
    default:
        return 0;
    }

    if (getRandom(0, 99) >= spawnChance) {
        return 0;
    }
    return getRandom(minTraps, maxTraps);
}

} // namespace

// Constructor
Room::Room()
    : roomId(0), difficulty(0), roomType(NORMAL),
      hasShop(false), isCleared(false), goblinNest(false) {}

Room::~Room() = default;

// initialization of room
void Room::initRoom(int id, int diff, RoomType type)
{
    roomId = id;
    difficulty = diff;
    roomType = type;
    isCleared = false;
    goblinNest = false;
    hasShop = (type == SHOP);
}

// room generation logic based on difficulty
void Room::generateRoomContent(int diff)
{
    monsters.clear();
    traps.clear();
    goblinNest = false;

    // Variables for monster and trap counts
    int monsterCount = 0;
    int trapCount = 0;
    EncounterKind encounter = EncounterKind::GOBLIN;

    // Determine counts based on room type and difficulty
    switch (roomType)
    {
    case NORMAL:
        switch (diff)
        {
        case 0:
            monsterCount = getRandom(1, 2);
            break;
        case 1:
            monsterCount = getRandom(1, 3);
            break;
        default:
            monsterCount = getRandom(1, 3);
            break;
        }
        trapCount = rollTrapCount(diff, roomType);
        break;

    case BOSS:
        monsterCount = 1;
        trapCount = rollTrapCount(diff, roomType);
        break;

    case SHOP:
        // Shop rooms: no monsters/traps
        monsterCount = 0;
        trapCount = 0;
        break;

    case TREASURE:
        monsterCount = getRandom(0, 1);
        trapCount = rollTrapCount(diff, roomType);
        break;
    }

    if (monsterCount > 0 && roomType != SHOP) {
        encounter = rollEncounterKind(diff, roomType);
        if (encounter == EncounterKind::GOBLIN_NEST) {
            monsterCount = 3;
            goblinNest = true;
        }
    }

    // Create monsters
    for (int i = 0; i < monsterCount; i++)
    {
        std::unique_ptr<Monster> monster(new Monster());
        spawnMonster(monster.get(), diff, encounter);
        monsters.push_back(std::move(monster));
    }

    for (int i = 0; i < trapCount; i++)
    {
        std::unique_ptr<Trap> trap(new Trap());

        // Randomly select trap type
        TrapType type = TrapType::SPIKE_PIT;
        int trapRoll = getRandom(0, 3);
        switch (trapRoll)
        {
        case 0:
            type = TrapType::SPIKE_PIT;
            break;
        case 1:
            type = TrapType::POISON_GAS;
            break;
        case 2:
            type = TrapType::FALLING_BLOCK;
            break;
        case 3:
            type = TrapType::FIRE_BLAST;
            break;
        default:
            break;
        }

        trap->initTrap(type, diff);
        traps.push_back(std::move(trap));
    }

    // Update room description based on content
    std::ostringstream desc;
    switch (roomType)
    {
    case NORMAL:
        if (goblinNest) {
            desc << "A foul Goblin Nest teeming with " << monsterCount << " goblins.";
        } else {
            desc << "A dimly lit chamber with " << monsterCount << " enemies lurking.";
        }
        break;
    case BOSS:
        desc << "A grand hall. A powerful foe awaits!";
        break;
    case SHOP:
        desc << "A quiet room with a merchant's stall.";
        break;
    case TREASURE:
        desc << "A glimmering treasure room. Riches await!";
        break;
    }
    description = desc.str();
}

// Display room information
std::string Room::showRoomInfo() const
{
    std::ostringstream ss;
    ss << "Room ID: " << roomId << "\n";
    ss << "Difficulty: " << difficulty << "\n";
    ss << "Type: ";
    switch (roomType)
    {
    case NORMAL:
        ss << "Normal";
        break;
    case BOSS:
        ss << "Boss";
        break;
    case SHOP:
        ss << "Shop";
        break;
    case TREASURE:
        ss << "Treasure";
        break;
    }
    ss << "\nMonsters: " << monsters.size() << "\n";
    ss << "Traps: " << traps.size() << "\n";
    ss << "Shop: " << (hasShop ? "Yes" : "No") << "\n";
    ss << "Cleared: " << (isCleared ? "Yes" : "No") << "\n";
    return ss.str();
}

// Room management
bool Room::clearRoom()
{
    isCleared = true;
    return true;
}

bool Room::isRoomCleared() const
{
    return isCleared;
}

bool Room::hasShopInRoom() const
{
    return hasShop;
}

// Getters
RoomType Room::getRoomType() const
{
    return roomType;
}

int Room::getRoomId() const
{
    return roomId;
}

int Room::getDifficulty() const
{
    return difficulty;
}

const std::vector<std::unique_ptr<Monster>>& Room::getMonsters() const
{
    return monsters;
}

const std::vector<std::unique_ptr<Trap>>& Room::getTraps() const
{
    return traps;
}

std::string Room::getDescription() const
{
    return description;
}

bool Room::isGoblinNestRoom() const
{
    return goblinNest;
}

// Setters
void Room::setCleared(bool cleared)
{
    isCleared = cleared;
}

void Room::setHasShop(bool shop)
{
    hasShop = shop;
}

void Room::setRoomType(RoomType type)
{
    roomType = type;
}