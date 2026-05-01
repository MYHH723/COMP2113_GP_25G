// types.h
#ifndef TYPES_H
#define TYPES_H

#include <string>

enum RoomType { NORMAL, BOSS, SHOP, TREASURE };
enum ItemType { POTION, WEAPON, ARMOR };
enum ItemRarity { LOW, MEDIUM, HIGH };
enum Difficulty { EASY, NORMAL, HARD };

// 声明字符串数组（定义在 types.cpp 中）
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