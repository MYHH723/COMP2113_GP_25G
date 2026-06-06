#include "merchant.h"
#include "item.h"
#include "types.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

namespace {

const char* const kHawk[] = {
    "Fresh ware from caravan and crypt alike!",
    "Prices fair as moonlight - feel the weight of each piece.",
    "Names mean little; coin names everything.",
    "See here - steel that remembers honour, draught that remembers life.",
    "Blades sharpened, potions stirred - all before compline bell!",
    "From distant fiefs and nearer graves - goods for the bold!",
    "No haggling with spectres, but mortals may yet bargain.",
    "Quality fit for a squire's purse or a baron's pride.",
    "Smell the oil on the leather; hear the truth in the steel.",
    "What the dungeon breaks, I mend - for a modest tithe.",
};

void mutterMerchantLine() {
    const size_t idx = static_cast<size_t>(
        getRandom(0, static_cast<int>(sizeof(kHawk) / sizeof(kHawk[0])) - 1));
    std::cout << "  Merchant: \"" << kHawk[idx] << "\"\n";
}

const char* rarityLabel(ItemRarity rarity) {
    switch (rarity) {
    case LOW: return "Low";
    case MEDIUM: return "Medium";
    case HIGH: return "High";
    default: return "?";
    }
}

} // namespace

Merchant::Merchant()
    : isAvailable(true), currentDiff(1), nextItemId(10000) {
    initMerchant();
}

Merchant::Merchant(int gameDiff, int seed)
    : isAvailable(true),
      currentDiff(std::max(0, std::min(2, gameDiff))),
      nextItemId(10000 + (std::max(0, seed) % 100000) * 10) {
    initMerchant();
}

Merchant::~Merchant() = default;

void Merchant::initMerchant() {
    goodsByGrade.clear();
    restockGoods(POTION, LOW);
    restockGoods(POTION, MEDIUM);
    restockGoods(POTION, HIGH);
    restockGoods(WEAPON, LOW);
    restockGoods(WEAPON, MEDIUM);
    restockGoods(WEAPON, HIGH);
    restockGoods(ARMOR, LOW);
    restockGoods(ARMOR, MEDIUM);
    restockGoods(ARMOR, HIGH);
}

bool Merchant::hasItem(ItemType type, int grade) const {
    if (grade < 0 || grade > 2) return false;
    const auto it = goodsByGrade.find(type);
    return it != goodsByGrade.end() && it->second[static_cast<size_t>(grade)] != 0;
}

Item Merchant::getItem(ItemType type, int grade) {
    const int id = goodsByGrade.at(type)[static_cast<size_t>(grade)];
    return Item(id);
}

void Merchant::showGoodsList() const {
    mutterMerchantLine();
    std::cout << "\n===== Merchant Shop =====" << std::endl;
    for (const auto& pair : goodsByGrade) {
        for (int grade = 0; grade < 3; ++grade) {
            const int id = pair.second[static_cast<size_t>(grade)];
            if (id == 0) continue;
            Item temp(id);
            std::cout << "- [" << rarityLabel(temp.getRarity()) << "] "
                      << temp.getName() << " (+" << static_cast<int>(temp.getEffectValue())
                      << ", " << temp.getPrice() << " gold)" << std::endl;
        }
    }
    std::cout << "=========================" << std::endl;
}

void Merchant::restockGoods(ItemType type, ItemRarity rarity) {
    const int grade = static_cast<int>(rarity);
    const int id = nextItemId++;

    std::string name;
    int cost = 0;
    float effectValue = 0.0f;

    switch (type) {
    case POTION:
        name = std::string(rarityLabel(rarity)) + " Potion";
        if (rarity == LOW) { effectValue = 12.0f; cost = 15; }
        else if (rarity == MEDIUM) { effectValue = 16.0f; cost = 20; }
        else { effectValue = 22.0f; cost = 28; }
        break;
    case WEAPON:
        name = std::string(rarityLabel(rarity)) + " Sword";
        if (rarity == LOW) { effectValue = 5.0f; cost = 18; }
        else if (rarity == MEDIUM) { effectValue = 10.0f; cost = 25; }
        else { effectValue = 15.0f; cost = 35; }
        break;
    case ARMOR:
        name = std::string(rarityLabel(rarity)) + " Armor";
        if (rarity == LOW) { effectValue = 5.0f; cost = 16; }
        else if (rarity == MEDIUM) { effectValue = 10.0f; cost = 22; }
        else { effectValue = 15.0f; cost = 30; }
        break;
    }

    itemDatabase[id] = {name, type, rarity, effectValue, cost, false};
    goodsByGrade[type][static_cast<size_t>(grade)] = id;
}

bool Merchant::getIsAvailable() const { return isAvailable; }
void Merchant::setIsAvailable(bool state) { isAvailable = state; }
int Merchant::getCurrentDiff() const { return currentDiff; }

void seedMerchantCatalog(int gameDiff, int seed) {
    Merchant merchant(gameDiff, seed);
    (void)merchant;
}
