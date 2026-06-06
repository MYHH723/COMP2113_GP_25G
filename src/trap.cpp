#include "trap.h"
#include "player.h"
#include "balance.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <algorithm>

namespace {

const char* pickRandomLine(const char* const* lines, size_t count) {
    if (count == 0) return "";
    const size_t idx = static_cast<size_t>(getRandom(0, static_cast<int>(count) - 1));
    return lines[idx];
}

const char* const kSpikePitDesc[] = {
    "Spikes suddenly emerge from the flagstones!",
    "The floor gives way to iron teeth hungry for blood!",
    "A pit yawns open - spears greet you like unwelcome hosts.",
    "Hidden springs snap; the ground becomes a garden of steel.",
    "Wrought spikes thrust upward, relics of a cruel architect.",
};

const char* const kPoisonGasDesc[] = {
    "Poisonous vapour seeps from cracks in the masonry!",
    "Green mist rolls along the floor, sweet and deadly.",
    "A hiss - alchemical fumes claim the very air you need.",
    "The chamber exhales venom; your lungs protest.",
    "Ancient bellows release a witch's breath upon you.",
};

const char* const kFallingBlockDesc[] = {
    "Rocks thunder down from the vaulted ceiling!",
    "Masonry breaks loose - the hall tries to bury you alive.",
    "A grinding roar; stone rain punishes the unwary.",
    "Timbers snap; the ceiling gifts you crushing weight.",
    "Dust and boulders fall like judgement from above.",
};

const char* const kFireBlastDesc[] = {
    "Fire erupts from hidden vents in the wall!",
    "A jet of flame roars across the stones like dragon's breath.",
    "Oil ignites; the corridor becomes a brief inferno.",
    "Brass nozzles spit hellfire - some long-dead hand designed this.",
    "Heat blooms without warning; mail glows, flesh remembers.",
};

} // namespace

// Default constructor: Initialize trap attributes
Trap::Trap() 
    : trapType(TrapType::SPIKE_PIT), damage_min(0), damage_max(0), 
      trigger_count(0), isActive(true), description("") {
}

// Destructor
Trap::~Trap() {
}

// Initialize trap type, damage, and description based on difficulty
void Trap::initTrap(TrapType type, int difficulty) {
    (void)difficulty;
    trapType = type;
    trigger_count = 0;
    isActive = true;

    switch (type) {
        case TrapType::SPIKE_PIT:
            description = pickRandomLine(kSpikePitDesc, sizeof(kSpikePitDesc) / sizeof(kSpikePitDesc[0]));
            break;
        case TrapType::POISON_GAS:
            description = pickRandomLine(kPoisonGasDesc, sizeof(kPoisonGasDesc) / sizeof(kPoisonGasDesc[0]));
            break;
        case TrapType::FALLING_BLOCK:
            description = pickRandomLine(kFallingBlockDesc, sizeof(kFallingBlockDesc) / sizeof(kFallingBlockDesc[0]));
            break;
        case TrapType::FIRE_BLAST:
            description = pickRandomLine(kFireBlastDesc, sizeof(kFireBlastDesc) / sizeof(kFireBlastDesc[0]));
            break;
    }

    float typeScale = 1.0f;
    switch(type) {
        case TrapType::SPIKE_PIT: typeScale = 1.0f; break;
        case TrapType::POISON_GAS: typeScale = 0.7f; break;
        case TrapType::FALLING_BLOCK: typeScale = 1.2f; break;
        case TrapType::FIRE_BLAST: typeScale = 0.85f; break;
    }

    const int baseMin = g_trapDamageMin > 0 ? g_trapDamageMin : 5;
    const int baseMax = g_trapDamageMax > 0 ? g_trapDamageMax : 12;
    damage_min = std::max(1, static_cast<int>(baseMin * typeScale));
    damage_max = std::max(damage_min, static_cast<int>(baseMax * typeScale));
}

// Getter methods for trap attributes
TrapType Trap::getTrapType() const { return trapType; }
int Trap::getDamageMin() const { return damage_min; }
int Trap::getDamageMax() const { return damage_max; }
int Trap::getTriggerCount() const { return trigger_count; }
bool Trap::get_isActive() const { return isActive; }
std::string Trap::getDescription() const { return description; }

// Setter methods for trap state
void Trap::setActive(bool active) { isActive = active; }
void Trap::setTriggerCount(int count) { trigger_count = count; }

// Activate trap and return random damage value
int Trap::activateTrap() {
    if (!isActive) return 0;
    
    trigger_count++;
    return getRandom(damage_min, damage_max);
}

// Trigger trap and apply damage + poison chance to player
void Trap::triggerTrap(Player& player) {
    if (!isActive) return;
    
    int damage = activateTrap();
    player.change_HP(-static_cast<float>(damage));
    isActive = false;

    if (getRandomChance(30)) player.set_isPoisoned(true);
}

// Return formatted trap information string
std::string Trap::getTrapInfo() const {
    std::string typeStr;
    switch(trapType) {
        case TrapType::SPIKE_PIT: typeStr = "Spike Trap"; break;
        case TrapType::POISON_GAS: typeStr = "Poison Gas Trap"; break;
        case TrapType::FALLING_BLOCK: typeStr = "Falling Block Trap"; break;
        case TrapType::FIRE_BLAST: typeStr = "Fire Blast Trap"; break;
    }
    
    std::string info = "Trap Type: " + typeStr + "\n";
    info += "Description: " + description + "\n";
    info += "Damage Range: " + std::to_string(damage_min) + " - " + 
            std::to_string(damage_max) + "\n";
    info += "Trigger Count: " + std::to_string(trigger_count) + "\n";
    info += "Status: " + std::string(isActive ? "Active" : "Inactive") + "\n";
    
    return info;
}
