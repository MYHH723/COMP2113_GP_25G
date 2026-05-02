// types.h
#ifndef TYPES_H
#define TYPES_H

#include <string>

// Enumeration for room types in the game map
enum RoomType { NORMAL, BOSS, SHOP, TREASURE };

// Enumeration for item categories
enum ItemType { POTION, WEAPON, ARMOR };

// Enumeration for item quality levels
enum ItemRarity { LOW, MEDIUM, HIGH };

// Enumeration for game difficulty settings
enum Difficulty { DIFF_EASY, DIFF_NORMAL, DIFF_HARD };

// External string arrays for item names (defined in types.cpp)
extern const std::string lowPotion[7];
extern const std::string mediumPotion[7];
extern const std::string highPotion[10];
extern const std::string LOW_WEAPON_NAMES[10];
extern const std::string MEDIUM_WEAPON_NAMES[10];
extern const std::string HIGH_WEAPON_NAMES[10];
extern const std::string LOW_ARMOR_NAMES[10];
extern const std::string MEDIUM_ARMOR_NAMES[10];
extern const std::string HIGH_ARMOR_NAMES[10];

#endif
