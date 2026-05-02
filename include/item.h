// item.h
#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <unordered_map>
#include "types.h"

// Forward declaration
class Player;

/**
 * @struct ItemData
 * @brief Data structure to store all core attributes of an item
 */
struct ItemData {
    std::string name;
    ItemType type;
    ItemRarity rarity;
    float effectValue;
    int price;
    bool isConsumed;
};

/**
 * @brief Global item database that maps item ID to ItemData
 */
extern std::unordered_map<int, ItemData> itemDatabase;

/**
 * @brief Initializes the global item database with default items
 * @return None
 */
void initItemDatabase();

/**
 * @class Item
 * @brief Item class that uses ID to reference item data from the database
 */
class Item {
private:
    int id;

public:
    /**
     * @brief Default constructor
     */
    Item();

    /**
     * @brief Constructor with item ID
     * @param id - Unique integer ID of the item
     */
    explicit Item(int id);           

    /**
     * @brief Gets the item ID
     * @return int - Item ID
     */
    int getId() const;

    /**
     * @brief Gets the item name from the database
     * @return std::string - Item name
     */
    std::string getName() const;

    /**
     * @brief Gets the item type (WEAPON, ARMOR, POTION, etc.)
     * @return ItemType - Type of the item
     */
    ItemType getType() const;

    /**
     * @brief Gets the item rarity (LOW, MEDIUM, HIGH)
     * @return ItemRarity - Rarity level
     */
    ItemRarity getRarity() const;

    /**
     * @brief Gets the effect value of the item
     * @return float - Numeric effect value
     */
    float getEffectValue() const;

    /**
     * @brief Gets the item price
     * @return int - Item price in gold
     */
    int getPrice() const;

    /**
     * @brief Checks if the item is consumed on use
     * @return bool - True if consumed, false otherwise
     */
    bool getIsConsumed() const;

    /**
     * @brief Sets whether the item is consumed when used
     * @param state - Boolean consumption state
     * @return None
     */
    void setIsConsumed(bool state);

    /**
     * @brief Applies the item's effect to the player
     * @param player - Reference to the player object
     * @return None
     */
    void applyEffect(Player& player);

    /**
     * @brief Displays complete item information to the console
     * @return None
     */
    void displayItemInfo() const;

    /**
     * @brief Equality operator to compare two items by ID
     * @param other - Another Item object to compare
     * @return bool - True if IDs are equal
     */
    bool operator==(const Item& other) const;
};

#endif
