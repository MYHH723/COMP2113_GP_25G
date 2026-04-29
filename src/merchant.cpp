#include "merchant.h"
#include "item.h"
#include "types.h"
#include <iostream>
#include <random>
#include <ctime>
#include <chrono>
#include <string>
#include <unordered_set>

std::mt19937 gen(std::random_device{}());
std::mt19937 rng;

static int generateUniqueId() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<int> dist(1, 2'000'000'000);
    static std::unordered_set<int> usedIds;
    
    int newId;
    do {
        newId = dist(gen);
    } while (usedIds.find(newId) != usedIds.end());
    
    usedIds.insert(newId);
    return newId;
}

// Constructor: Initialize merchant with game difficulty and set default availability
Merchant::Merchant(int gameDiff,int)
    : currentDiff(gameDiff), isAvailable(true) {
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);
    initMerchant();
}

// Destructor
Merchant::~Merchant() = default;

// Initialize all merchant goods with 3 rarity levels per item type
void Merchant::initMerchant() {
    // Initialize Consumable Items (Potion)
    restockGoods(POTION);
    restockGoods(WEAPON);
    restockGoods(ARMOR);
}

// Check if the merchant has the specified item type and index
bool Merchant::hasItem(ItemType type) const {
    if(goods.at(type) == 0) {
        return false;
    }
    return true;
}

// Get the item from goods by type and index
Item Merchant::getItem(ItemType type){
    if (!hasItem(type)) {
        restockGoods(type); 
    }
    Item item = Item(goods.at(type));
    goods.at(type) = 0; // Remove item from stock
    return item;

}

// Show all goods list with detailed information
void Merchant::showGoodsList() const {
    std::cout << "\n===== Merchant Shop =====" << std::endl;

    // Display all consumable items
    std::cout << "\n[Consumable Items]: ";
    Item(goods.at(POTION)).displayItemInfo();

    // Display all weapon items
    std::cout << "[Weapon Items]: ";
    Item(goods.at(WEAPON)).displayItemInfo();

    // Display all armor items
    std::cout << "[Armor Items]: ";
    Item(goods.at(ARMOR)).displayItemInfo();

}

void Merchant::restockGoods(ItemType type) {
    int id;
    do{
        id = generateUniqueId();
        Item(id);
    }while(itemDatabase[id].type != type);
    goods.at(type) = id;
}

// Get merchant availability status
bool Merchant::getIsAvailable() const {
    return isAvailable;
}

// Set merchant availability status
void Merchant::setIsAvailable(bool state) {
    isAvailable = state;
}

// Get current game difficulty
int Merchant::getCurrentDiff() const {
    return currentDiff;
}
