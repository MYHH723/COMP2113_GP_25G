#include "monster.h"
#include "player.h"
#include "balance.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <cmath>

// Default constructor: Initialize monster attributes to default values
Monster::Monster() 
    : name(""), hp(0), maxHp(0), atk(0), def(0), 
      exp_reward(0), gold_reward(0), score_reward(0.0f), level(1), isAlive(true) {
}

// Destructor
Monster::~Monster() {
}

// Initialize monster stats based on game difficulty and level
void Monster::initMonster(int difficulty, int monsterLevel) {
    level = monsterLevel;
    (void)difficulty;

    const float hpMult = g_monsterHpMultiplier > 0.0f ? g_monsterHpMultiplier : 1.0f;

    // Set monster type and stats based on level (HP/ATK scaled by g_monsterHpMultiplier)
    if (level <= 2) {
        name = "Goblin";
        maxHp = static_cast<int>(30 * hpMult * (1 + 0.1 * level));
        atk = static_cast<int>(8 * hpMult * (1 + 0.1 * level));
        def = static_cast<int>(2 * hpMult);
        exp_reward = 10 + level * 5;
        gold_reward = 5 + level * 3;
        score_reward = 1.0f + level * 0.5f;
    } else if (level <= 4) {
        name = "Skeleton Warrior";
        maxHp = static_cast<int>(40 * hpMult * (1 + 0.1 * level));
        atk = static_cast<int>(10 * hpMult * (1 + 0.1 * level));
        def = static_cast<int>(3 * hpMult);
        exp_reward = 15 + level * 5;
        gold_reward = 8 + level * 3;
        score_reward = 2.0f + level * 1.0f;
    } else if (level <= 6) {
        name = "Dark Mage";
        maxHp = static_cast<int>(35 * hpMult * (1 + 0.1 * level));
        atk = static_cast<int>(15 * hpMult * (1 + 0.1 * level));
        def = static_cast<int>(1 * hpMult);
        exp_reward = 20 + level * 5;
        gold_reward = 10 + level * 3;
        score_reward = 4.0f + level * 2.0f;
    } else {
        name = "Dungeon Lord";
        maxHp = static_cast<int>(100 * hpMult * (1 + 0.1 * level));
        atk = static_cast<int>(20 * hpMult * (1 + 0.1 * level));
        def = static_cast<int>(5 * hpMult);
        exp_reward = 50 + level * 10;
        gold_reward = 30 + level * 5;
        score_reward = 8.0f + level * 4.0f;
    }
    
    hp = maxHp;
    isAlive = true;
}

// Getter methods for monster attributes
std::string Monster::getName() const { return name; }
int Monster::getHP() const { return hp; }
int Monster::getMaxHP() const { return maxHp; }
int Monster::getATK() const { return atk; }
int Monster::getDEF() const { return def; }
int Monster::getExpReward() const { return exp_reward; }
int Monster::getGoldReward() const { return gold_reward; }
float Monster::getScoreReward() const { return score_reward; }
int Monster::getLevel() const { return level; }
bool Monster::get_isAlive() const { return isAlive; }

// Set monster HP with bounds checking
void Monster::setHP(int new_hp) {
    hp = new_hp;
    if (hp < 0) hp = 0;
    if (hp > maxHp) hp = maxHp;
    isAlive = (hp > 0);
}

// Set monster attack value with bounds checking
void Monster::setATK(int new_atk) {
    atk = new_atk;
    if (atk < 0) atk = 0;
}

// Set monster defense value with bounds checking
void Monster::setDEF(int new_def) {
    def = new_def;
    if (def < 0) def = 0;
}

// Set monster alive status
void Monster::set_isAlive(bool alive) { isAlive = alive; }

// Apply damage to monster (DEF already applied by BattleSystem::playerAttack)
void Monster::takeDamage(int damage) {
    if (!isAlive) return;

    int actualDamage = damage;
    if (actualDamage < 1) actualDamage = 1;

    hp -= actualDamage;
    if (hp < 0) hp = 0;
    
    if (hp == 0) isAlive = false;
}

// Monster attacks player and returns actual damage dealt
int Monster::attackPlayer(Player& player) {
    if (!isAlive) return 0;
    
    const int baseDamage = getRandom(
        static_cast<int>(atk * 0.8),
        static_cast<int>(atk * 1.2));
    
    float playerDef = player.get_DEF();
    int actualDamage = baseDamage - static_cast<int>(playerDef);
    if (actualDamage < 1) actualDamage = 1;
    
    player.change_HP(-static_cast<float>(actualDamage));
    
    return actualDamage;
}

// Mark monster as dead and set HP to 0
void Monster::die() {
    isAlive = false;
    hp = 0;
}

// Return formatted monster information string
std::string Monster::getMonsterInfo() const {
    std::string info = "Monster: " + name + "\n";
    info += "Level: " + std::to_string(level) + "\n";
    info += "HP: " + std::to_string(hp) + "/" + std::to_string(maxHp) + "\n";
    info += "ATK: " + std::to_string(atk) + "\n";
    info += "DEF: " + std::to_string(def) + "\n";
    info += "Status: " + std::string(isAlive ? "Alive" : "Dead") + "\n";
    info += "EXP Reward: " + std::to_string(exp_reward) + "\n";
    info += "Gold Reward: " + std::to_string(gold_reward) + "\n";
    
    return info;
}
