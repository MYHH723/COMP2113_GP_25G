#include "player.h"
#include "savegame.h"
#include "item.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <algorithm>

#include "third_party/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

// Inventory implementation
Inventory::Inventory() : capacity(MAX_INVENTORY_SIZE) {}

Inventory::~Inventory() {}

bool Inventory::add_item(const int id)
{
    if (items.size() < capacity)
    {
        items.push_back(id);
        sort_items();
        return true;
    }
    return false;
}

bool Inventory::remove_item(const int id)
{
    auto it = std::find(items.begin(), items.end(), id);
    if (it != items.end())
    {
        items.erase(it);
        return true;
    }
    return false;
}

int Inventory::get_item(const int id) {
    if(itemDatabase.find(id) != itemDatabase.end()) {
        return id;

    }
    return 0;
}

bool Inventory::use_item(const int id)
{
    // Simple implementation: remove on use
    return remove_item(id);
}

void Inventory::sort_items()
{
    // Define type priorities using enum values
    std::map<ItemType, int> typePriority = {
        {ItemType::WEAPON, 3},
        {ItemType::ARMOR, 2},
        {ItemType::POTION, 1}};

    items.sort([&typePriority, this](const int &idA, const int &idB)
               {
        // Get type from itemDatabase using id (type is ItemType enum)
        ItemType typeA = itemDatabase[idA].type;
        ItemType typeB = itemDatabase[idB].type;

        // First compare by type priority (higher first)
        int priA = typePriority.count(typeA) ? typePriority[typeA] : 0;
        int priB = typePriority.count(typeB) ? typePriority[typeB] : 0;
        if (priA != priB) {
            return priA > priB;
        }
        
        // Then by grade (if exists) or other attributes
        // return itemDatabase[idA].grade > itemDatabase[idB].grade;
        
        return false; });
}

int Inventory::get_capacity() const
{
    return capacity;
}

int Inventory::get_current_size() const
{
    return items.size();
}




// Player implementation
Player::Player(std::string name) : playerName(name)
{
    state["LEVEL"] = 1;
    state["LEVEL"] = 1;
    state["ATK"] = DEFAULT_ATK;
    state["DEF"] = DEFAULT_DEF;
    state["HP"] = DEFAULT_HP;
    state["EXP"] = DEFAULT_EXP;
    state["Money"] = DEFAULT_MONEY;
    maxHP = DEFAULT_HP;
    inventory = new Inventory();
    maxHP = DEFAULT_HP;
    equippedItems["WEAPON"] = 0;
    equippedItems["ARMOR"] = 0;
    score = 0.0f;
    isAlive = true;

}

Player::~Player()
{
    delete inventory;
}

std::map<std::string, float> Player::get_state() const
{
    return state;
}

int Player::get_Level() const
{
    return static_cast<int>(state.at("LEVEL"));
}

float Player::get_ATK() const
{
    return state.at("ATK");
}

float Player::get_DEF() const
{
    return state.at("DEF");
}

float Player::get_HP() const
{
    return state.at("HP");
}

float Player::get_EXP() const
{
    return state.at("EXP");
}

float Player::get_Money() const
{
    return state.at("Money");
}

float Player::get_maxHP() const
{
    return maxHP;
}

bool Player::get_isAlive() const
{
    return isAlive;
}

float Player::get_score() const
{
    return score + get_Level() * 20 + get_ATK() * 5 + get_DEF() * 5 + get_EXP() * 0.1f + get_Money() * 0.01f;
}

std::list<int> Player::get_all_items() const{
    return inventory->get_items();
}

void Player::change_state(const std::string &key, float value)
{
    state[key] = value;
}

void Player::change_ATK(float amount)
{
    state["ATK"] += amount;
}

void Player::change_DEF(float amount)
{
    state["DEF"] += amount;
}

void Player::change_HP(float amount)
{
    state["HP"] += amount;
    if (state["HP"] > maxHP)
    {
        state["HP"] = maxHP;
    }
    if (state["HP"] > maxHP)
    {
        state["HP"] = maxHP;
    }
    if (state["HP"] <= 0)
    {
        isAlive = false;
    }
}

void Player::change_EXP(float amount)
{
    state["EXP"] += amount;
    int level = state.at("LEVEL");
    float exp = state.at("EXP");
    while(exp >= 100 * std::pow(1.1, level - 1)) {
        exp -= 100 * std::pow(1.1, level - 1); 
        level++;
        level_up();
    }
    state["LEVEL"] = level;
    state["EXP"] = exp;
}

void Player::change_Money(float amount)
{
    state["Money"] += amount;
}

void Player::change_score(float amount) {
    score += amount;
}

void Player::set_isPoisoned(bool poisoned) {
    // 如果不需要 poison 机制，可以空实现或添加成员变量
    // 这里简单加一个成员变量（需要在类中添加 bool isPoisoned;）
    // 为了不改变结构，暂时空实现
    (void)poisoned;
}

void Player::set_isAlive(bool alive)
{
    isAlive = alive;
}

void Player::add_item(const int id)
{
    inventory->add_item(id);
}

void Player::remove_item(const int id)
{
    inventory->remove_item(id);
}

void Player::use_item(const int id)
{
    inventory->use_item(id);
}

void Player::sort_items()
{
    inventory->sort_items();
}

void Player::level_up()
{
    std::cout << "Level Up! You are now level " << get_Level() << "!" << std::endl;
    for(int time = get_Level() + 1; time > 0; time--) {
        std::cout << "Choose an attribute to increase: " << "(remaining " << time << " points)" << std::endl;
        std::cout << "1. ATK" << std::endl;
        std::cout << "2. DEF" << std::endl;
        std::cout << "3. HP" << std::endl;

        std::string choice;
        while(std::cin >> choice) {
            if (choice == "1" || choice == "ATK" || choice == "2" || choice == "DEF" || choice == "3" || choice == "HP") {
                break;
            }
            std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
        }

        if (choice == "1" || choice == "ATK") {
            change_ATK(0.1*DEFAULT_ATK);
        }
        else if (choice == "2" || choice == "DEF") {
            change_DEF(0.1*DEFAULT_DEF);
        }
        else if (choice == "3" || choice == "HP") {
            maxHP += 0.1*DEFAULT_HP;
            change_HP(0.1*DEFAULT_HP);
        }
    }
    
}

void Player::equip(const int id)
{
    if (itemDatabase.find(id) == itemDatabase.end()) return;
    if (itemDatabase[id].type == WEAPON)
    {
        if(inventory->remove_item(id)){
            if (equippedItems["WEAPON"] != 0) {
                float oldEffect = itemDatabase[(equippedItems["WEAPON"])].effectValue;
                state["ATK"] -= oldEffect;
                inventory->add_item(equippedItems["WEAPON"]);
            }
            equippedItems["WEAPON"] = id;
            float newEffect = itemDatabase[id].effectValue;
            state["ATK"] += newEffect;
        }
    }
    else if (itemDatabase[id].type == ARMOR)
    {
        if(inventory->remove_item(id)){
            if (equippedItems["ARMOR"] != 0) {
                float oldEffect = itemDatabase[(equippedItems["ARMOR"])].effectValue;
                state["DEF"] -= oldEffect;
                inventory->add_item(equippedItems["ARMOR"]);
            }
            equippedItems["ARMOR"] = id;
            float newEffect = itemDatabase[id].effectValue;
            state["DEF"] += newEffect;
        }
    }

    

}



std::string Player::itemToString(const Item &item)
{
    std::string typeStr;
    switch (item.getType())
    {
    case ItemType::POTION:
        typeStr = "POTION";
        break;
    case ItemType::WEAPON:
        typeStr = "WEAPON";
        break;
    case ItemType::ARMOR:
        typeStr = "ARMOR";
        break;
    }
    return typeStr + ":" + std::to_string(item.getRarity()) + ":" + item.getName() + ":" +
           std::to_string(item.getEffectValue()) + ":" + std::to_string(item.getPrice());
}

json Player::toJson() const {
    json j;
    
    // === Stats (from state map) ===
    j["stats"]["LEVEL"] = get_Level();
    j["stats"]["ATK"] = get_ATK();
    j["stats"]["DEF"] = get_DEF();
    j["stats"]["HP"] = get_HP();
    j["stats"]["EXP"] = get_EXP();
    j["stats"]["Money"] = get_Money();
    j["stats"]["maxHP"] = maxHP;
    
    // === Status flags ===
    j["status"]["isAlive"] = isAlive;
    j["status"]["score"] = score;
    
    // === Inventory (preserve your exact id) ===
    j["inventory"] = json::array();
    for (const int& itemStr : inventory->get_items()) {
        j["inventory"].push_back(itemStr);  // "SWORD:2:Iron Blade:25:150:150:false"
    }

    j["equipped"]["WEAPON"] = equippedItems.count("WEAPON") ? equippedItems.at("WEAPON") : 0;
    j["equipped"]["ARMOR"] = equippedItems.count("ARMOR") ? equippedItems.at("ARMOR") : 0;
    
    return j;
}

void Player::fromJson(const json& j) {
    // === Load stats ===
    if (j.contains("stats")) {
        const auto& stats = j["stats"];
        if (stats.contains("LEVEL")) state["LEVEL"] = stats["LEVEL"].get<float>();
        if (stats.contains("ATK")) state["ATK"] = stats["ATK"].get<float>();
        if (stats.contains("DEF")) state["DEF"] = stats["DEF"].get<float>();
        if (stats.contains("HP")) {
            state["HP"] = stats["HP"].get<float>();
            if (state["HP"] <= 0) isAlive = false;  // Sync alive flag
        }
        if (stats.contains("EXP")) state["EXP"] = stats["EXP"].get<float>();
        if (stats.contains("Money")) state["Money"] = stats["Money"].get<float>();
        if (stats.contains("maxHP")) maxHP = stats["maxHP"].get<float>();
    }
    
    // === Load status flags ===
    if (j.contains("status")) {
        const auto& status = j["status"];
        if (status.contains("isAlive")) isAlive = status["isAlive"].get<bool>();
        if (status.contains("score")) score = status["score"].get<float>();
    }
    
    // === Load inventory (clear first, then restore) ===
    if (j.contains("inventory") && j["inventory"].is_array()) {
        inventory->clear_items();  
        for (const auto& itemId : j["inventory"]) {
            int id = itemId.get<int>();
            Item temp(id);
            inventory->add_item(id);
        }
    }
    
    // === Load equipped items ===
    if (j.contains("equipped")) {
        const auto& equipped = j["equipped"];
        if (equipped.contains("WEAPON")) {
            equippedItems["WEAPON"] = equipped["WEAPON"];
            Item temp(equippedItems["WEAPON"]);
        }
        if (equipped.contains("ARMOR")) {
            equippedItems["ARMOR"] = equipped["ARMOR"];
            Item temp(equippedItems["ARMOR"]);
        }
    }
}