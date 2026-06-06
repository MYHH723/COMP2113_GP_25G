#include "player.h"
#include "item.h"
#include "utils.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <cstring>

#include "third_party/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

const char SLOT_WEAPON[] = "WEAPON";
const char SLOT_WEAPON2[] = "WEAPON2";
const char SLOT_ARMOR[] = "ARMOR";
const char SLOT_ARMOR2[] = "ARMOR2";
const char SLOT_ARMOR3[] = "ARMOR3";

namespace {

bool isWeaponSlot(const char* slot) {
    return std::strcmp(slot, SLOT_WEAPON) == 0 || std::strcmp(slot, SLOT_WEAPON2) == 0;
}

bool isArmorSlot(const char* slot) {
    return std::strcmp(slot, SLOT_ARMOR) == 0 || std::strcmp(slot, SLOT_ARMOR2) == 0 ||
           std::strcmp(slot, SLOT_ARMOR3) == 0;
}

const char* const kWeaponSlots[] = {SLOT_WEAPON, SLOT_WEAPON2};
const char* const kArmorSlots[] = {SLOT_ARMOR, SLOT_ARMOR2, SLOT_ARMOR3};
const size_t kWeaponSlotCount = 2;
const size_t kArmorSlotCount = 3;

} // namespace

namespace {

const char* pickPoisonLine(const char* const* lines, size_t count) {
    if (count == 0) return "";
    const size_t idx = static_cast<size_t>(getRandom(0, static_cast<int>(count) - 1));
    return lines[idx];
}

const char* const kPoisonApplied[] = {
    "You feel venom spreading through your veins like winter!",
    "A sickly heat crawls beneath your skin - you are poisoned.",
    "Your limbs grow heavy; some foul brew has found its mark.",
    "The air you breathed was treachery; poison takes its toll.",
    "Bitterness floods your blood; the apothecary's nightmare awakens.",
};

const char* const kPoisonTick[] = {
    "Poison gnaws at your flesh (HP: ",
    "Venom saps your strength (HP: ",
    "The toxin burns inward (HP: ",
    "Bile and fever wrack you (HP: ",
    "Dark humours claim their due (HP: ",
};

} // namespace

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

bool Inventory::contains(const int id) const
{
    return std::find(items.begin(), items.end(), id) != items.end();
}

int Inventory::count_item(const int id) const
{
    int count = 0;
    for (const int itemId : items) {
        if (itemId == id) ++count;
    }
    return count;
}

int Inventory::get_item(const int id)
{
    return contains(id) ? id : 0;
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
    inventory.reset(new Inventory());
    equippedItems[SLOT_WEAPON] = 0;
    equippedItems[SLOT_WEAPON2] = 0;
    equippedItems[SLOT_ARMOR] = 0;
    equippedItems[SLOT_ARMOR2] = 0;
    equippedItems[SLOT_ARMOR3] = 0;
    score = 0.0f;
    isAlive = true;
    isPoisoned = false;
}

// Player destructor: Free inventory memory
Player::~Player() = default;

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
float Player::get_battleScore() const
{
    return score;
}

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

// Poison status: apply DOT and persist until cured
void Player::set_isPoisoned(bool poisoned) {
    isPoisoned = poisoned;
    if (poisoned) {
        std::cout << "  * "
                  << pickPoisonLine(kPoisonApplied, sizeof(kPoisonApplied) / sizeof(kPoisonApplied[0]))
                  << "\n";
    }
}

bool Player::get_isPoisoned() const {
    return isPoisoned;
}

void Player::tickPoison() {
    if (!isPoisoned || !isAlive) return;
    change_HP(-3.0f);
    if (isAlive) {
        std::cout << "  * "
                  << pickPoisonLine(kPoisonTick, sizeof(kPoisonTick) / sizeof(kPoisonTick[0]))
                  << static_cast<int>(get_HP()) << ").\n";
    }
}

void Player::curePoison() {
    if (!isPoisoned) return;
    isPoisoned = false;
    std::cout << "  * The poison fades from your body.\n";
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

// Level up system: one attribute point per level gained
void Player::level_up()
{
    std::cout << "Level Up! You are now level " << get_Level() << "!" << std::endl;
    std::cout << "Choose one attribute to increase:" << std::endl;
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
        if (choice == "1" || choice == "ATK" || choice == "2" || choice == "DEF" ||
            choice == "3" || choice == "HP") {
            discardRestOfLine();
            break;
        }
        discardRestOfLine();
        std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
    }

    if (choice == "1" || choice == "ATK") {
        change_ATK(0.1 * DEFAULT_ATK);
    } else if (choice == "2" || choice == "DEF") {
        change_DEF(0.1 * DEFAULT_DEF);
    } else if (choice == "3" || choice == "HP") {
        maxHP += 0.1 * DEFAULT_HP;
        change_HP(0.1 * DEFAULT_HP);
    }
}

int Player::getEquippedId(const std::string& slot) const
{
    const auto it = equippedItems.find(slot);
    return it != equippedItems.end() ? it->second : 0;
}

int Player::getEquippedId(const char* slot) const
{
    if (slot == nullptr) return 0;
    return getEquippedId(std::string(slot));
}

bool Player::isDualWielding() const
{
    return getEquippedId(SLOT_WEAPON2) != 0;
}

int Player::countEquippedArmor() const
{
    int count = 0;
    if (getEquippedId(SLOT_ARMOR) != 0) ++count;
    if (getEquippedId(SLOT_ARMOR2) != 0) ++count;
    if (getEquippedId(SLOT_ARMOR3) != 0) ++count;
    return count;
}

int Player::getEquippedArmorCount() const
{
    return countEquippedArmor();
}

bool Player::isFullLayeredArmor() const
{
    return countEquippedArmor() >= MAX_LAYERED_ARMOR;
}

const char* Player::firstEmptyArmorSlot() const
{
    if (getEquippedId(SLOT_ARMOR) == 0) return SLOT_ARMOR;
    if (getEquippedId(SLOT_ARMOR2) == 0) return SLOT_ARMOR2;
    if (getEquippedId(SLOT_ARMOR3) == 0) return SLOT_ARMOR3;
    return nullptr;
}

bool Player::stashAllArmor(const char* logPrefix)
{
    const char* slots[] = {SLOT_ARMOR, SLOT_ARMOR2, SLOT_ARMOR3};
    const int toStash = countEquippedArmor();
    if (toStash == 0) return true;
    if (inventory->get_current_size() + toStash > inventory->get_capacity()) return false;

    bool stashed = false;
    for (const char* slot : slots) {
        const int armorId = getEquippedId(slot);
        if (armorId == 0) continue;
        inventory->add_item(armorId);
        change_DEF(-itemDatabase[armorId].effectValue);
        equippedItems[slot] = 0;
        stashed = true;
    }
    if (stashed && logPrefix != nullptr) {
        std::cout << logPrefix << '\n';
    }
    return true;
}

bool Player::stashSecondWeaponForArmor(const char* logPrefix)
{
    const int weapon2Id = getEquippedId(SLOT_WEAPON2);
    if (weapon2Id == 0) return true;
    if (!inventory->add_item(weapon2Id)) return false;
    change_ATK(-itemDatabase[weapon2Id].effectValue);
    equippedItems[SLOT_WEAPON2] = 0;
    if (logPrefix != nullptr) {
        std::cout << logPrefix << '\n';
    }
    return true;
}

int Player::countOwnedItems(int itemId) const
{
    if (itemId == 0) return 0;
    int count = 0;
    for (const int id : inventory->get_items()) {
        if (id == itemId) ++count;
    }
    if (getEquippedId(SLOT_WEAPON) == itemId) ++count;
    if (getEquippedId(SLOT_WEAPON2) == itemId) ++count;
    if (getEquippedId(SLOT_ARMOR) == itemId) ++count;
    if (getEquippedId(SLOT_ARMOR2) == itemId) ++count;
    if (getEquippedId(SLOT_ARMOR3) == itemId) ++count;
    return count;
}

namespace {

int countEquippedSlotsWithItem(const Player& player, int itemId) {
    int count = 0;
    for (size_t i = 0; i < kWeaponSlotCount; ++i) {
        if (player.getEquippedId(kWeaponSlots[i]) == itemId) ++count;
    }
    for (size_t i = 0; i < kArmorSlotCount; ++i) {
        if (player.getEquippedId(kArmorSlots[i]) == itemId) ++count;
    }
    return count;
}

const char* findUniqueEquippedSlot(const Player& player, int itemId) {
    const char* found = nullptr;
    int count = 0;
    for (size_t i = 0; i < kWeaponSlotCount; ++i) {
        if (player.getEquippedId(kWeaponSlots[i]) != itemId) continue;
        found = kWeaponSlots[i];
        ++count;
    }
    for (size_t i = 0; i < kArmorSlotCount; ++i) {
        if (player.getEquippedId(kArmorSlots[i]) != itemId) continue;
        found = kArmorSlots[i];
        ++count;
    }
    return count == 1 ? found : nullptr;
}

float sumEquippedWeaponEffect(const Player& player) {
    float sum = 0.0f;
    for (size_t i = 0; i < kWeaponSlotCount; ++i) {
        const int id = player.getEquippedId(kWeaponSlots[i]);
        if (id == 0) continue;
        const auto it = itemDatabase.find(id);
        if (it != itemDatabase.end()) sum += it->second.effectValue;
    }
    return sum;
}

float sumEquippedArmorEffect(const Player& player) {
    float sum = 0.0f;
    for (size_t i = 0; i < kArmorSlotCount; ++i) {
        const int id = player.getEquippedId(kArmorSlots[i]);
        if (id == 0) continue;
        const auto it = itemDatabase.find(id);
        if (it != itemDatabase.end()) sum += it->second.effectValue;
    }
    return sum;
}

} // namespace

bool Player::hasInventorySpace() const
{
    return inventory->get_current_size() < inventory->get_capacity();
}

bool Player::unequipSlot(const char* slot, bool addToInventory)
{
    if (slot == nullptr) return false;
    const int id = getEquippedId(slot);
    if (id == 0) return false;
    if (itemDatabase.find(id) == itemDatabase.end()) return false;
    if (addToInventory && !hasInventorySpace()) return false;
    if (addToInventory && !inventory->add_item(id)) return false;

    const float effect = itemDatabase[id].effectValue;
    if (isWeaponSlot(slot)) {
        state["ATK"] -= effect;
    } else if (isArmorSlot(slot)) {
        state["DEF"] -= effect;
    }
    equippedItems[slot] = 0;

    if (addToInventory) {
        Item item(id);
        std::cout << "[Equip] Unequipped " << item.getName() << " to backpack.\n";
    }
    return true;
}

bool Player::unequipItem(int id, bool addToInventory)
{
    if (itemDatabase.find(id) == itemDatabase.end()) return false;

    const char* slot = findUniqueEquippedSlot(*this, id);
    if (slot == nullptr) {
        if (countEquippedSlotsWithItem(*this, id) > 1) {
            std::cout << "[Equip] Same item in multiple slots — use Unequip and pick a slot.\n";
        }
        return false;
    }

    return unequipSlot(slot, addToInventory);
}

bool Player::usePotionFromInventory(int id) {
    if (itemDatabase.find(id) == itemDatabase.end()) return false;
    if (itemDatabase[id].type != POTION) return false;
    if (!inventory->contains(id)) return false;

    const float heal = itemDatabase[id].effectValue;
    inventory->remove_item(id);
    change_HP(heal > 0.0f ? heal : static_cast<float>(getRandom(10, 23)));

    if (get_isPoisoned()) {
        curePoison();
        std::cout << "[Inventory] The draught eases the poison from your veins.\n";
    }
    std::cout << "[Inventory] Used potion: +"
              << static_cast<int>(heal > 0.0f ? heal : 0) << " HP.\n";
    return true;
}

bool Player::equipFromInventory(int id) {
    if (!inventory->contains(id)) return false;
    if (itemDatabase.find(id) == itemDatabase.end()) return false;
    const ItemType type = itemDatabase[id].type;
    if (type != WEAPON && type != ARMOR) return false;
    equip(id);
    return !inventory->contains(id);
}

void Player::equip(const int id)
{
    if (itemDatabase.find(id) == itemDatabase.end()) return;
    const ItemType type = itemDatabase[id].type;
    const float effect = itemDatabase[id].effectValue;

    if (type == WEAPON) {
        if (!inventory->remove_item(id)) return;

        if (equippedItems[SLOT_WEAPON] == 0) {
            equippedItems[SLOT_WEAPON] = id;
            state["ATK"] += effect;
            return;
        }

        if (equippedItems[SLOT_WEAPON2] == 0) {
            if (isFullLayeredArmor()) {
                inventory->add_item(id);
                std::cout << "[Equip] Triple armor: only one sword allowed.\n";
                return;
            }
            if (countEquippedArmor() > 0 && !stashAllArmor("[Equip] Dual-wield: all armor stowed in backpack.")) {
                inventory->add_item(id);
                return;
            }
            equippedItems[SLOT_WEAPON2] = id;
            state["ATK"] += effect;
            std::cout << "[Equip] Dual-wield: second sword equipped.\n";
            return;
        }

        const int oldId = equippedItems[SLOT_WEAPON2];
        state["ATK"] -= itemDatabase[oldId].effectValue;
        inventory->add_item(oldId);
        equippedItems[SLOT_WEAPON2] = id;
        state["ATK"] += effect;
        return;
    }

    if (type == ARMOR) {
        if (!inventory->remove_item(id)) return;

        if (!stashSecondWeaponForArmor("[Equip] Layered armor: second sword stowed in backpack.")) {
            inventory->add_item(id);
            return;
        }

        const char* emptySlot = firstEmptyArmorSlot();
        if (emptySlot != nullptr) {
            equippedItems[emptySlot] = id;
            state["DEF"] += effect;
            if (isFullLayeredArmor()) {
                std::cout << "[Equip] Triple armor active (single sword only).\n";
            }
            return;
        }

        const int oldId = equippedItems[SLOT_ARMOR3];
        if (!inventory->add_item(oldId)) {
            inventory->add_item(id);
            return;
        }
        state["DEF"] -= itemDatabase[oldId].effectValue;
        equippedItems[SLOT_ARMOR3] = id;
        state["DEF"] += effect;
    }
}

// Equip freshly purchased gear without requiring a free inventory slot first
bool Player::purchaseAndEquip(int id)
{
    if (itemDatabase.find(id) == itemDatabase.end()) return false;
    const ItemType type = itemDatabase[id].type;
    if (type != WEAPON && type != ARMOR) return false;

    if (inventory->contains(id)) {
        equip(id);
        return true;
    }

    const float effect = itemDatabase[id].effectValue;

    if (type == WEAPON) {
        if (equippedItems[SLOT_WEAPON] == 0) {
            equippedItems[SLOT_WEAPON] = id;
            state["ATK"] += effect;
            return true;
        }

        if (equippedItems[SLOT_WEAPON2] == 0) {
            if (isFullLayeredArmor()) {
                std::cout << "[Shop] Triple armor: only one sword allowed.\n";
                return false;
            }
            if (countEquippedArmor() > 0 &&
                !stashAllArmor("[Shop] Dual-wield: all armor stowed in backpack.")) {
                return false;
            }
            equippedItems[SLOT_WEAPON2] = id;
            state["ATK"] += effect;
            std::cout << "[Shop] Dual-wield: second sword equipped.\n";
            return true;
        }

        const int oldId = equippedItems[SLOT_WEAPON2];
        if (!inventory->add_item(oldId)) return false;
        state["ATK"] -= itemDatabase[oldId].effectValue;
        equippedItems[SLOT_WEAPON2] = id;
        state["ATK"] += effect;
        return true;
    }

    if (!stashSecondWeaponForArmor("[Shop] Layered armor: second sword stowed in backpack.")) {
        return false;
    }

    const char* emptySlot = firstEmptyArmorSlot();
    if (emptySlot != nullptr) {
        equippedItems[emptySlot] = id;
        state["DEF"] += effect;
        if (isFullLayeredArmor()) {
            std::cout << "[Shop] Triple armor active (single sword only).\n";
        }
        return true;
    }

    const int oldId = equippedItems[SLOT_ARMOR3];
    if (!inventory->add_item(oldId)) return false;
    state["DEF"] -= itemDatabase[oldId].effectValue;
    equippedItems[SLOT_ARMOR3] = id;
    state["DEF"] += effect;
    return true;
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
    j["status"]["isPoisoned"] = isPoisoned;
    j["status"]["score"] = score;

    j["inventory"] = json::array();
    for (const int& itemStr : inventory->get_items()) {
        j["inventory"].push_back(itemStr);
    }

    j["equipped"]["WEAPON"] = equippedItems.count(SLOT_WEAPON) ? equippedItems.at(SLOT_WEAPON) : 0;
    j["equipped"]["WEAPON2"] = equippedItems.count(SLOT_WEAPON2) ? equippedItems.at(SLOT_WEAPON2) : 0;
    j["equipped"]["ARMOR"] = equippedItems.count(SLOT_ARMOR) ? equippedItems.at(SLOT_ARMOR) : 0;
    j["equipped"]["ARMOR2"] = equippedItems.count(SLOT_ARMOR2) ? equippedItems.at(SLOT_ARMOR2) : 0;
    j["equipped"]["ARMOR3"] = equippedItems.count(SLOT_ARMOR3) ? equippedItems.at(SLOT_ARMOR3) : 0;

    std::unordered_set<int> referencedIds;
    for (const int itemId : inventory->get_items()) {
        referencedIds.insert(itemId);
    }
    const int weaponId = getEquippedId(SLOT_WEAPON);
    const int weapon2Id = getEquippedId(SLOT_WEAPON2);
    const int armorId = getEquippedId(SLOT_ARMOR);
    const int armor2Id = getEquippedId(SLOT_ARMOR2);
    const int armor3Id = getEquippedId(SLOT_ARMOR3);
    if (weaponId != 0) referencedIds.insert(weaponId);
    if (weapon2Id != 0) referencedIds.insert(weapon2Id);
    if (armorId != 0) referencedIds.insert(armorId);
    if (armor2Id != 0) referencedIds.insert(armor2Id);
    if (armor3Id != 0) referencedIds.insert(armor3Id);

    j["itemDefs"] = json::array();
    for (const int itemId : referencedIds) {
        const auto it = itemDatabase.find(itemId);
        if (it == itemDatabase.end()) continue;
        const ItemData& data = it->second;
        json entry;
        entry["id"] = itemId;
        entry["name"] = data.name;
        entry["type"] = static_cast<int>(data.type);
        entry["rarity"] = static_cast<int>(data.rarity);
        entry["effectValue"] = data.effectValue;
        entry["price"] = data.price;
        j["itemDefs"].push_back(entry);
    }

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
        if (status.contains("isPoisoned") && status["isPoisoned"].is_boolean())
            isPoisoned = status["isPoisoned"].get<bool>();
        if (status.contains("score") && status["score"].is_number())
            score = status["score"].get<float>();
    }

    static const int kMaxItemId = 50000000;

    if (j.contains("itemDefs") && j["itemDefs"].is_array()) {
        for (const auto& entry : j["itemDefs"]) {
            if (!entry.is_object()) continue;
            if (!entry.contains("id") || !entry["id"].is_number_integer()) continue;
            const int itemId = entry["id"].get<int>();
            if (itemId <= 0 || itemId > kMaxItemId) continue;

            ItemData data;
            data.name = entry.contains("name") && entry["name"].is_string()
                            ? entry["name"].get<std::string>()
                            : "Unknown";
            data.type = entry.contains("type") && entry["type"].is_number_integer()
                            ? static_cast<ItemType>(entry["type"].get<int>())
                            : POTION;
            data.rarity = entry.contains("rarity") && entry["rarity"].is_number_integer()
                              ? static_cast<ItemRarity>(entry["rarity"].get<int>())
                              : LOW;
            data.effectValue = entry.contains("effectValue") && entry["effectValue"].is_number()
                                   ? entry["effectValue"].get<float>()
                                   : 0.0f;
            data.price = entry.contains("price") && entry["price"].is_number_integer()
                             ? entry["price"].get<int>()
                             : 0;
            data.isConsumed = false;
            itemDatabase[itemId] = data;
        }
    }

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
                equippedItems[SLOT_WEAPON] = wid;
                Item temp(wid);
            }
        }
        if (equipped.contains("WEAPON2") && equipped["WEAPON2"].is_number_integer()) {
            const int w2id = equipped["WEAPON2"].get<int>();
            if (w2id >= 0 && w2id <= kMaxItemId) {
                equippedItems[SLOT_WEAPON2] = w2id;
                Item temp(w2id);
            }
        } else {
            equippedItems[SLOT_WEAPON2] = 0;
        }
        if (equipped.contains("ARMOR") && equipped["ARMOR"].is_number_integer()) {
            const int aid = equipped["ARMOR"].get<int>();
            if (aid >= 0 && aid <= kMaxItemId) {
                equippedItems[SLOT_ARMOR] = aid;
                Item temp(aid);
            }
        }
        if (equipped.contains("ARMOR2") && equipped["ARMOR2"].is_number_integer()) {
            const int a2id = equipped["ARMOR2"].get<int>();
            if (a2id >= 0 && a2id <= kMaxItemId) {
                equippedItems[SLOT_ARMOR2] = a2id;
                Item temp(a2id);
            }
        } else {
            equippedItems[SLOT_ARMOR2] = 0;
        }
        if (equipped.contains("ARMOR3") && equipped["ARMOR3"].is_number_integer()) {
            const int a3id = equipped["ARMOR3"].get<int>();
            if (a3id >= 0 && a3id <= kMaxItemId) {
                equippedItems[SLOT_ARMOR3] = a3id;
                Item temp(a3id);
            }
        } else {
            equippedItems[SLOT_ARMOR3] = 0;
        }
    }

    reconcileEquipmentAfterLoad();
}

void Player::reconcileEquipmentAfterLoad()
{
    float weaponEffect = sumEquippedWeaponEffect(*this);
    const float savedAtk = state["ATK"];
    float coreAtk = savedAtk - weaponEffect;

    if (getEquippedId(SLOT_WEAPON2) != 0 && coreAtk < DEFAULT_ATK - 0.01f) {
        const int w2 = getEquippedId(SLOT_WEAPON2);
        const auto it = itemDatabase.find(w2);
        if (it != itemDatabase.end()) {
            equippedItems[SLOT_WEAPON2] = 0;
            weaponEffect -= it->second.effectValue;
            coreAtk = savedAtk - weaponEffect;
        }
    }
    if (coreAtk < DEFAULT_ATK - 0.01f) {
        coreAtk = DEFAULT_ATK;
    }
    state["ATK"] = coreAtk + sumEquippedWeaponEffect(*this);

    float armorEffect = sumEquippedArmorEffect(*this);
    const float savedDef = state["DEF"];
    float coreDef = savedDef - armorEffect;

    const char* stripOrder[] = {SLOT_ARMOR3, SLOT_ARMOR2, SLOT_ARMOR};
    while (coreDef < DEFAULT_DEF - 0.01f && getEquippedArmorCount() > 0) {
        bool stripped = false;
        for (const char* slot : stripOrder) {
            const int armorId = getEquippedId(slot);
            if (armorId == 0) continue;
            const auto it = itemDatabase.find(armorId);
            if (it == itemDatabase.end()) continue;
            equippedItems[slot] = 0;
            armorEffect -= it->second.effectValue;
            coreDef = savedDef - armorEffect;
            stripped = true;
            break;
        }
        if (!stripped) break;
    }
    if (coreDef < DEFAULT_DEF - 0.01f) {
        coreDef = DEFAULT_DEF;
    }
    state["DEF"] = coreDef + sumEquippedArmorEffect(*this);
}
