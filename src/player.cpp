#include "player.h"
#include "savegame.h"
#include "item.h"
#include "utils.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <algorithm>

#include "third_party/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

// Inventory constructor: Initialize with maximum capacity
Inventory::Inventory() : capacity(MAX_INVENTORY_SIZE) {}

// Inventory destructor
Inventory::~Inventory() {}

// Add item to inventory if capacity allows
bool Inventory::add_item(const int id)
{
    if (items.size() < static_cast<std::list<int>::size_type>(capacity))
    {
        items.push_back(id);
        sort_items();
        return true;
    }
    return false;
}

// Remove specified item from inventory
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

// Get item ID if it exists in database
int Inventory::get_item(const int id) {
    if(itemDatabase.find(id) != itemDatabase.end()) {
        return id;
    }
    return 0;
}

// Use item (simplified: remove from inventory)
bool Inventory::use_item(const int id)
{
    return remove_item(id);
}

// Sort inventory items by type priority (WEAPON > ARMOR > POTION)
void Inventory::sort_items()
{
    std::map<ItemType, int> typePriority = {
        {ItemType::WEAPON, 3},
        {ItemType::ARMOR, 2},
        {ItemType::POTION, 1}};

    items.sort([&typePriority](const int &idA, const int &idB) {
        const auto itA = itemDatabase.find(idA);
        const auto itB = itemDatabase.find(idB);
        if (itA == itemDatabase.end() && itB == itemDatabase.end())
            return idA < idB;
        if (itA == itemDatabase.end()) return false;
        if (itB == itemDatabase.end()) return true;

        const ItemType typeA = itA->second.type;
        const ItemType typeB = itB->second.type;

        const int priA = typePriority.count(typeA) ? typePriority[typeA] : 0;
        const int priB = typePriority.count(typeB) ? typePriority[typeB] : 0;
        if (priA != priB) return priA > priB;
        return false;
    });
}

// Get maximum inventory capacity
int Inventory::get_capacity() const
{
    return capacity;
}

// Get current number of items in inventory
int Inventory::get_current_size() const
{
    return items.size();
}

// Player constructor: Initialize stats and inventory
Player::Player(std::string name) : playerName(name)
{
    state["LEVEL"] = 1;
    state["ATK"] = DEFAULT_ATK;
    state["DEF"] = DEFAULT_DEF;
    state["HP"] = DEFAULT_HP;
    state["EXP"] = DEFAULT_EXP;
    state["Money"] = DEFAULT_MONEY;
    maxHP = DEFAULT_HP;
    inventory = new Inventory();
    equippedItems["WEAPON"] = 0;
    equippedItems["ARMOR"] = 0;
    score = 0.0f;
    isAlive = true;
}

// Player destructor: Free inventory memory
Player::~Player()
{
    delete inventory;
}

// Get complete player state map
std::map<std::string, float> Player::get_state() const
{
    return state;
}

// Get player level
int Player::get_Level() const
{
    return static_cast<int>(state.at("LEVEL"));
}

// Get player attack value
float Player::get_ATK() const
{
    return state.at("ATK");
}

// Get player defense value
float Player::get_DEF() const
{
    return state.at("DEF");
}

// Get player current HP
float Player::get_HP() const
{
    return state.at("HP");
}

// Get player current EXP
float Player::get_EXP() const
{
    return state.at("EXP");
}

// Get player money
float Player::get_Money() const
{
    return state.at("Money");
}

// Get player maximum HP
float Player::get_maxHP() const
{
    return maxHP;
}

// Check if player is alive
bool Player::get_isAlive() const
{
    return isAlive;
}

// Calculate total player score based on stats
float Player::get_score() const
{
    return score + get_Level() * 20 + get_ATK() * 5 + get_DEF() * 5 + get_EXP() * 0.1f + get_Money() * 0.01f;
}

// Get all items from player inventory
std::list<int> Player::get_all_items() const{
    return inventory->get_items();
}

// Modify specific player state value
void Player::change_state(const std::string &key, float value)
{
    state[key] = value;
}

// Modify player attack value
void Player::change_ATK(float amount)
{
    state["ATK"] += amount;
}

// Modify player defense value
void Player::change_DEF(float amount)
{
    state["DEF"] += amount;
}

// Modify player HP (capped at maxHP, sets alive status)
void Player::change_HP(float amount)
{
    state["HP"] += amount;
    if (state["HP"] > maxHP)
    {
        state["HP"] = maxHP;
    }
    if (state["HP"] <= 0)
    {
        isAlive = false;
    }
}

// Modify player EXP and handle automatic level-up
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

// Modify player money
void Player::change_Money(float amount)
{
    state["Money"] += amount;
}

// Modify player score
void Player::change_score(float amount) {
    score += amount;
}

// Placeholder for poison status (empty implementation)
void Player::set_isPoisoned(bool poisoned) {
    (void)poisoned;
}

// Set player alive status
void Player::set_isAlive(bool alive)
{
    isAlive = alive;
}

// Add item to player inventory
void Player::add_item(const int id)
{
    inventory->add_item(id);
}

// Remove item from player inventory
void Player::remove_item(const int id)
{
    inventory->remove_item(id);
}

// Use item from inventory
void Player::use_item(const int id)
{
    inventory->use_item(id);
}

// Sort items in player inventory
void Player::sort_items()
{
    inventory->sort_items();
}

// Level up system: prompt player to increase stats
void Player::level_up()
{
    std::cout << "Level Up! You are now level " << get_Level() << "!" << std::endl;
    for(int time = get_Level() + 1; time > 0; time--) {
        std::cout << "Choose an attribute to increase: " << "(remaining " << time << " points)" << std::endl;
        std::cout << "1. ATK" << std::endl;
        std::cout << "2. DEF" << std::endl;
        std::cout << "3. HP" << std::endl;

        std::string choice;
        while (true) {
            if (!(std::cin >> choice)) {
                std::cin.clear();
                discardRestOfLine();
                std::cout << "Invalid input. Enter 1, 2, or 3." << std::endl;
                continue;
            }
            if (choice == "1" || choice == "ATK" || choice == "2" || choice == "DEF" || choice == "3" || choice == "HP") {
                discardRestOfLine();
                break;
            }
            discardRestOfLine();
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

// Equip weapon/armor and swap with current equipment
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

// Convert item details to formatted string
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

// Serialize player data to JSON format
json Player::toJson() const {
    json j;

    j["stats"]["LEVEL"] = get_Level();
    j["stats"]["ATK"] = get_ATK();
    j["stats"]["DEF"] = get_DEF();
    j["stats"]["HP"] = get_HP();
    j["stats"]["EXP"] = get_EXP();
    j["stats"]["Money"] = get_Money();
    j["stats"]["maxHP"] = maxHP;

    j["status"]["isAlive"] = isAlive;
    j["status"]["score"] = score;

    j["inventory"] = json::array();
    for (const int& itemStr : inventory->get_items()) {
        j["inventory"].push_back(itemStr);
    }

    j["equipped"]["WEAPON"] = equippedItems.count("WEAPON") ? equippedItems.at("WEAPON") : 0;
    j["equipped"]["ARMOR"] = equippedItems.count("ARMOR") ? equippedItems.at("ARMOR") : 0;

    return j;
}

// Deserialize player data from JSON format
void Player::fromJson(const json& j) {
    if (j.contains("stats") && j["stats"].is_object()) {
        const auto& stats = j["stats"];
        if (stats.contains("LEVEL") && stats["LEVEL"].is_number())
            state["LEVEL"] = stats["LEVEL"].get<float>();
        if (stats.contains("ATK") && stats["ATK"].is_number())
            state["ATK"] = stats["ATK"].get<float>();
        if (stats.contains("DEF") && stats["DEF"].is_number())
            state["DEF"] = stats["DEF"].get<float>();
        if (stats.contains("HP") && stats["HP"].is_number()) {
            state["HP"] = stats["HP"].get<float>();
            if (state["HP"] <= 0) isAlive = false;
        }
        if (stats.contains("EXP") && stats["EXP"].is_number())
            state["EXP"] = stats["EXP"].get<float>();
        if (stats.contains("Money") && stats["Money"].is_number())
            state["Money"] = stats["Money"].get<float>();
        if (stats.contains("maxHP") && stats["maxHP"].is_number())
            maxHP = stats["maxHP"].get<float>();
    }

    if (j.contains("status") && j["status"].is_object()) {
        const auto& status = j["status"];
        if (status.contains("isAlive") && status["isAlive"].is_boolean())
            isAlive = status["isAlive"].get<bool>();
        if (status.contains("score") && status["score"].is_number())
            score = status["score"].get<float>();
    }

    static const int kMaxItemId = 50000000;
    if (j.contains("inventory") && j["inventory"].is_array()) {
        inventory->clear_items();
        for (const auto& itemId : j["inventory"]) {
            if (!itemId.is_number_integer()) continue;
            const int id = itemId.get<int>();
            if (id <= 0 || id > kMaxItemId) continue;
            Item temp(id);
            if (!inventory->add_item(id)) break;
        }
    }

    if (j.contains("equipped") && j["equipped"].is_object()) {
        const auto& equipped = j["equipped"];
        if (equipped.contains("WEAPON") && equipped["WEAPON"].is_number_integer()) {
            const int wid = equipped["WEAPON"].get<int>();
            if (wid >= 0 && wid <= kMaxItemId) {
                equippedItems["WEAPON"] = wid;
                Item temp(wid);
            }
        }
        if (equipped.contains("ARMOR") && equipped["ARMOR"].is_number_integer()) {
            const int aid = equipped["ARMOR"].get<int>();
            if (aid >= 0 && aid <= kMaxItemId) {
                equippedItems["ARMOR"] = aid;
                Item temp(aid);
            }
        }
    }
}
