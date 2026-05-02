#include "mapgenerator.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

/**
 * @brief Default constructor
 * Initializes map generator with default values
 */
MapGenerator::MapGenerator()
    : totalRooms(0), difficulty(1), shopFrequency(4) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

/**
 * @brief Constructor with custom seed
 * @param seed Random seed for map generation
 */
MapGenerator::MapGenerator(int seed)
    : totalRooms(0), difficulty(1), shopFrequency(4) {
    std::srand(static_cast<unsigned>(seed));
}

/**
 * @brief Destructor
 * Clears the room list (rooms are deleted by the Game class)
 */
MapGenerator::~MapGenerator() {
    generatedRooms.clear();
}

/**
 * @brief Initializes map generator parameters
 * @param numRooms Total number of rooms to generate
 * @param diff Game difficulty (0 = Easy, 1 = Normal, 2 = Hard)
 */
void MapGenerator::initMapGenerator(int numRooms, int diff) {
    totalRooms = std::max(1, numRooms);
    difficulty = std::max(0, std::min(diff, 2));
    shopFrequency = 5 - difficulty; // easy=5, normal=4, hard=3
    if (shopFrequency < 2) shopFrequency = 2;
}

/**
 * @brief Generates the full dungeon map with all rooms
 * Creates and configures all rooms based on total room count
 */
void MapGenerator::generateMap() {
    generatedRooms.clear();
    generatedRooms.reserve(static_cast<size_t>(totalRooms));

    for (int roomNumber = 1; roomNumber <= totalRooms; ++roomNumber) {
        Room* room = generateRoom(roomNumber, roomNumber);
        generatedRooms.push_back(room);
    }
}

/**
 * @brief Creates and initializes a single room
 * @param roomId Unique ID for the room
 * @param roomNumber Sequential room number in the dungeon
 * @return Pointer to the created Room object
 */
Room* MapGenerator::generateRoom(int roomId, int roomNumber) {
    Room* room = new Room();

    RoomType type = NORMAL;
    if (roomNumber >= totalRooms) {
        // Last room is always a boss room
        type = BOSS;
    } else {
        // Random room type distribution
        int roll = std::rand() % 100; // 0-99
        if (roll < 65) {
            type = NORMAL;     // 65%
        } else if (roll < 80) {
            type = SHOP;       // 15%
        } else if (roll < 92) {
            type = TREASURE;   // 12%
        } else {
            type = BOSS;       // 8%
        }
    }

    room->initRoom(roomId, difficulty, type);
    room->generateRoomContent(difficulty);
    room->setHasShop(type == SHOP);
    return room;
}

/**
 * @brief Sets room type and shop status for a given room
 * @param room Pointer to the room to modify
 * @param roomNumber Sequential number of the room
 */
void MapGenerator::determineRoomType(Room* room, int roomNumber) {
    if (room == nullptr) return;
    RoomType type = determineRoomType(roomNumber);
    room->setRoomType(type);
    room->setHasShop(type == SHOP);
}

/**
 * @brief Determines room type based on room number and rules
 * @param roomNumber Sequential number of the room
 * @return RoomType Calculated room type
 */
RoomType MapGenerator::determineRoomType(int roomNumber) {
    if (roomNumber == totalRooms) return TREASURE;
    if (roomNumber > 0 && roomNumber < totalRooms && (roomNumber % 5) == 0) return BOSS;
    if (shouldHaveShop(roomNumber)) return SHOP;
    return NORMAL;
}

/**
 * @brief Gets the list of all generated rooms
 * @return std::vector<Room*> List of room pointers
 */
std::vector<Room*> MapGenerator::getGeneratedRooms() const {
    return generatedRooms;
}

/**
 * @brief Gets total number of rooms to generate
 * @return int Total room count
 */
int MapGenerator::getTotalRooms() const {
    return totalRooms;
}

/**
 * @brief Gets current difficulty level
 * @return int Difficulty (0=Easy,1=Normal,2=Hard)
 */
int MapGenerator::getDifficulty() const {
    return difficulty;
}

/**
 * @brief Finds a room by its unique ID
 * @param roomId ID of the room to find
 * @return Room* Pointer to the room, nullptr if not found
 */
Room* MapGenerator::getRoomById(int roomId) {
    for (Room* room : generatedRooms) {
        if (room != nullptr && room->getRoomId() == roomId) return room;
    }
    return nullptr;
}

/**
 * @brief Checks if a room should contain a shop
 * @param roomNumber Sequential room number
 * @return bool True if shop should be present
 */
bool MapGenerator::shouldHaveShop(int roomNumber) {
    if (roomNumber <= 1 || roomNumber >= totalRooms) return false;
    if (shopFrequency <= 0) return false;
    return (roomNumber % shopFrequency) == 0;
}

/**
 * @brief Returns random monster count based on difficulty
 * @param diff Game difficulty
 * @return int Number of monsters for a room
 */
int MapGenerator::getRandomMonsterCount(int diff) {
    int clamped = std::max(0, std::min(diff, 2));
    switch (clamped) {
        case 0: return 1 + (std::rand() % 2); // 1-2
        case 1: return 1 + (std::rand() % 3); // 1-3
        default: return 2 + (std::rand() % 3); // 2-4
    }
}

/**
 * @brief Returns random trap count based on difficulty
 * @param diff Game difficulty
 * @return int Number of traps for a room
 */
int MapGenerator::getRandomTrapCount(int diff) {
    int clamped = std::max(0, std::min(diff, 2));
    switch (clamped) {
        case 0: return std::rand() % 2; // 0-1
        case 1: return std::rand() % 3; // 0-2
        default: return 1 + (std::rand() % 3); // 1-3
    }
}

/**
 * @brief Counts how many rooms exist of a given type
 * @param type Room type to count
 * @return int Number of matching rooms
 */
int MapGenerator::countRoomsByType(RoomType type) const {
    int count = 0;
    for (Room* room : generatedRooms) {
        if (room != nullptr && room->getRoomType() == type) ++count;
    }
    return count;
}
