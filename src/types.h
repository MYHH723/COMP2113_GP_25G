// types.h
#ifndef TYPES_H
#define TYPES_H

// 枚举类型
typedef enum { NORMAL, BOSS, SHOP, TREASURE } RoomType;
typedef enum { POTION, WEAPON, ARMOR } ItemType;
typedef enum { LOW, MEDIUM, HIGH } ItemRarity;
typedef enum { EASY, NORMAL, HARD } Difficulty;

// 字符串数组声明（定义在 types.c 中）
extern const char* lowPotion[7];
extern const char* mediumPotion[7];
extern const char* highPotion[10];
extern const char* LOW_WEAPON_NAMES[10];
extern const char* MEDIUM_WEAPON_NAMES[10];
extern const char* HIGH_WEAPON_NAMES[10];
extern const char* LOW_ARMOR_NAMES[10];
extern const char* MEDIUM_ARMOR_NAMES[10];
extern const char* HIGH_ARMOR_NAMES[10];

#endif