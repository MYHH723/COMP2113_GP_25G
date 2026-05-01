// item.cpp
#include "item.h"
#include "player.h"
#include <iostream>
#include <unordered_set>
#include <iomanip>
#include <random>

std::unordered_map<int, ItemData> itemDatabase;
static std::mt19937 gen(std::random_device{}());
std::uniform_int_distribution<> int_dist(0, 30);

void initItemDatabase() {}

Item::Item() : id(0) {}

Item::Item(int id) : id(id) {
    if (itemDatabase.find(id) != itemDatabase.end()) {
        return;
    }
    
    std::mt19937 rng(id); 
    std::uniform_int_distribution<int> typeDist(0, 2);
    std::uniform_int_distribution<int> rarityDist(0, 2);

    
    ItemType type = static_cast<ItemType>(typeDist(rng));
    
    int rarity = static_cast<int>(rarityDist(rng));
    
    int price = 0;
    float effectValue = 0.0f;
    
    std::string name;
    switch (type) {
        case POTION:
            if(ItemRarity(rarity) == LOW) {
                effectValue = (int_dist(rng) + 10)/2;
                price = (int_dist(rng) + 50)*0.2;
                name = lowPotion[id % 7];
            }
            else if(ItemRarity(rarity) == MEDIUM) {
                effectValue = (int_dist(rng) + 50)/2;
                price = (int_dist(rng) + 100)*0.5;
                name = mediumPotion[id % 7];
            }
            else {
                effectValue = (int_dist(rng) + 100)/2;
                price = (int_dist(rng) + 200);
                name = highPotion[id % 10];
            }
            break;
        case WEAPON:
            if(ItemRarity(rarity) == LOW) {
                effectValue = (int_dist(rng) + 10);
                price = (int_dist(rng) + 10)*0.1;
                name = LOW_WEAPON_NAMES[id % 10];
            }
            else if(ItemRarity(rarity) == MEDIUM) {
                effectValue = (int_dist(rng) + 50);
                price = (int_dist(rng) + 50)*0.15;
                name = MEDIUM_WEAPON_NAMES[id % 10];
            }
            else {
                effectValue = (int_dist(rng) + 100);
                price = (int_dist(rng) + 100)*0.2;
                name = HIGH_WEAPON_NAMES[id % 10];
            }
            break;
        case ARMOR:
            if(ItemRarity(rarity) == LOW) {
                effectValue = (int_dist(rng) + 10);
                price = (int_dist(rng) + 10)*0.1;
                name = LOW_ARMOR_NAMES[id % 10];
            }
            else if(ItemRarity(rarity) == MEDIUM) {
                effectValue = (int_dist(rng) + 50);
                price = (int_dist(rng) + 50)*0.15;
                name = MEDIUM_ARMOR_NAMES[id % 10];
            }
            else {
                effectValue = (int_dist(rng) + 100);
                price = (int_dist(rng) + 100)*0.2;
                name = HIGH_ARMOR_NAMES[id % 10];
            }
            break;
        default:
            name = "Unknown Item";
            break;
    }
    
    itemDatabase[id] = {name, type, ItemRarity(rarity), effectValue, price, false};
}


int Item::getId() const { return id; }

std::string Item::getName() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.name : "";
}

ItemType Item::getType() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.type : POTION;
}

ItemRarity Item::getRarity() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.rarity : LOW;
}

float Item::getEffectValue() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.effectValue : 0.0f;
}

int Item::getPrice() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.price : 0;
}

bool Item::getIsConsumed() const {
    auto it = itemDatabase.find(id);
    return it != itemDatabase.end() ? it->second.isConsumed : false;
}

void Item::setIsConsumed(bool state) {
    auto it = itemDatabase.find(id);
    if (it != itemDatabase.end()) {
        it->second.isConsumed = state;
    }
}

void Item::displayItemInfo() const {
    auto it = itemDatabase.find(id);
    if (it == itemDatabase.end()) {
        std::cout << "Invalid item!" << std::endl;
        return;
    }
    
    const auto& data = it->second;
    std::cout << std::left;
    std::cout << "Item: " << data.name << std::endl;
    
    std::cout << "Type: ";
    switch (data.type) {
        case POTION: std::cout << "POTION"; break;
        case WEAPON: std::cout << "WEAPON"; break;
        case ARMOR: std::cout << "ARMOR"; break;
    }
    std::cout << std::endl;
    
    std::cout << "Rarity: ";
    switch (data.rarity) {
        case LOW: std::cout << "Low"; break;
        case MEDIUM: std::cout << "Medium"; break;
        case HIGH: std::cout << "High"; break;
    }
    std::cout << std::endl;
    
    std::cout << "Price: " << data.price << std::endl;
    
    std::cout << "Effect: ";
    switch (data.type) {
        case POTION: std::cout << "Heal " << data.effectValue; break;
        case WEAPON: std::cout << "Attack +" << data.effectValue; break;
        case ARMOR: std::cout << "Defense +" << data.effectValue; break;
    }
    std::cout << std::endl;
}

void Item::applyEffect(Player& player) {
    auto it = itemDatabase.find(id);
    if (it == itemDatabase.end()) return;
    
    const auto& data = it->second;
    switch (data.type) {
        case POTION:
            player.change_HP(data.effectValue);
            break;
        case WEAPON:
            player.change_ATK(data.effectValue);
            break;
        case ARMOR:
            player.change_DEF(data.effectValue);
            break;
    }
    setIsConsumed(true);
}

bool Item::operator==(const Item& other) const {
    return id == other.id;
}