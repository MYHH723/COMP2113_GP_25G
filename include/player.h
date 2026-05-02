#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include "item.h"
#include "third_party/json/single_include/nlohmann/json.hpp"

// Forward declaration of Inventory class
class Inventory;

// Player class - manages player character attributes and inventory
class Player {
private:
    // Player's display name
    std::string playerName;

    // Core player stats: level, ATK, DEF, HP, EXP, Money
    std::map<std::string, float> state;

    // Pointer to player's inventory instance
    Inventory* inventory;

    // Currently equipped items storage
    std::map<std::string, int> equippedItems;

    // Player's total game score
    float score;

    // Maximum HP value of the player
    float maxHP;

    // Player alive status flag
    bool isAlive;

public:
    // Default constructor
    Player();

    // Constructor with player name
    Player(std::string name);

    // Destructor
    ~Player();

    // Getter methods for player state and stats
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

    // Setter and state modification methods
    void change_state(const std::string& key, float value);
    void change_ATK(float amount);
    void change_DEF(float amount);
    void change_HP(float amount);
    void change_EXP(float amount);
    void change_Money(float amount);
    void change_score(float amount);
    void set_isPoisoned(bool poisoned);
    void set_isAlive(bool alive);

    // Inventory management functions
    void add_item(const int id);
    void remove_item(const int id);
    void use_item(const int id);
    void sort_items();
    std::string itemToString(const Item& item);
    std::list<int> get_all_items() const;

    // Equipment and progression
    void equip(const int id);
    void level_up();

    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

// Inventory class - manages item storage and operations
class Inventory {
private:
    // List storing item IDs
    std::list<int> items;

    // Maximum inventory capacity
    int capacity;

public:
    // Constructor
    Inventory();

    // Destructor
    ~Inventory();

    // Get all items in inventory
    const std::list<int>& get_items() const { return items; }

    // Clear all items
    void clear_items() { items.clear(); }

    // Inventory operations
    bool add_item(const int id);
    bool remove_item(const int id);
    bool use_item(const int id);
    void sort_items();
    int get_item(const int id);

    // Capacity and size
    int get_capacity() const;
    int get_current_size() const;
};

// Default player stat values
const float DEFAULT_ATK = 10.0f;
const float DEFAULT_DEF = 10.0f;
const float DEFAULT_HP = 100.0f;
const float DEFAULT_EXP = 0.0f;
const float DEFAULT_MONEY = 100.0f;

// Maximum inventory size limit
const int MAX_INVENTORY_SIZE = 20;

#endif
