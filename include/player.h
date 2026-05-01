// player.h 开头部分
#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include "item.h"
#include "third_party/json/single_include/nlohmann/json.hpp"

// 前向声明 Inventory 类
class Inventory;

// Player class - manages player character attributes and inventory
class Player {
private:
    std::string playerName;
    std::map<std::string, float> state;  // Stores level, ATK, DEF, HP, EXP, Money
    Inventory* inventory;
    std::map<std::string, int> equippedItems; 
    float score;
    float maxHP; 
    bool isAlive;

public:
    Player();
    Player(std::string name);
    ~Player();

    // Getters
    std::map<std::string, float> get_state() const;
    int get_Level() const;
    float get_ATK() const;
    float get_DEF() const;
    float get_HP() const;
    float get_EXP() const;
    float get_Money() const;
    float get_maxHP() const;
    Inventory* get_inventory() const { return inventory; }
    bool get_isAlive() const;
    float get_score() const;

    // Setters / State modifiers
    void change_state(const std::string& key, float value);
    void change_ATK(float amount);
    void change_DEF(float amount);
    void change_HP(float amount);
    void change_EXP(float amount);
    void change_Money(float amount);
    void change_score(float amount);
    void set_isPoisoned(bool poisoned);
    void set_isAlive(bool alive);


    // Inventory operations
    void add_item(const int id);
    void remove_item(const int id);
    void use_item(const int id);
    void sort_items();
    std::string itemToString(const Item& item);
    std::list<int> get_all_items() const;

    // Other methods
    void equip(const int id);
    void level_up();
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);

};


// Inventory class - manages items
class Inventory {
private:
    std::list<int> items;
    int capacity;

public:
    Inventory();
    ~Inventory();

    const std::list<int>& get_items() const { return items; }
    void clear_items() { items.clear(); }
    bool add_item(const int id);
    bool remove_item(const int id);
    bool use_item(const int id);
    void sort_items();
    int get_item(const int id);
    int get_capacity() const;
    int get_current_size() const;
};

// Global constants
const float DEFAULT_ATK = 10.0f;
const float DEFAULT_DEF = 10.0f;
const float DEFAULT_HP = 100.0f;
const float DEFAULT_EXP = 0.0f;
const float DEFAULT_MONEY = 100.0f;
const int MAX_INVENTORY_SIZE = 20;

#endif // PLAYER_H
