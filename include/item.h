// item.h
#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <unordered_map>
#include "types.h"

// Forward declaration
class Player;

struct ItemData {
    std::string name;
    ItemType type;
    ItemRarity rarity;
    float effectValue;
    int price;
    bool isConsumed;
};

extern std::unordered_map<int, ItemData> itemDatabase;

void initItemDatabase();

// Item :only id
class Item {
private:
    int id;

public:
    Item();
    explicit Item(int id);           
    
    int getId() const;
    std::string getName() const;
    ItemType getType() const;
    ItemRarity getRarity() const;
    float getEffectValue() const;
    int getPrice() const;
    bool getIsConsumed() const;
    void setIsConsumed(bool state);
    
    void applyEffect(Player& player);
    void displayItemInfo() const;
    
    bool operator==(const Item& other) const;
};

#endif