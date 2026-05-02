#include "battlesystem.h"
#include "player.h"
#include "monster.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

static std::mt19937 gen(std::random_device{}());

BattleSystem::BattleSystem() 
    : player(nullptr), currentMonster(nullptr), round_count(0), 
      isBattleActive(false), playerDefending(false),
      lastResult(BattleResult::ONGOING), reward{0.0f, 0.0f, 0.0f} {
    battleLog.clear();
    battleLog.push_back("Battle system initialized");
}

BattleSystem::~BattleSystem() {
    player = nullptr;
    currentMonster = nullptr;
}

void BattleSystem::initBattle(Player* p, Monster* m) {
    player = p;
    currentMonster = m;
    round_count = 0;
    isBattleActive = true;
    playerDefending = false;
    lastResult = BattleResult::ONGOING;
    reward[0] = 0.0f;
    reward[1] = 0.0f;
    reward[2] = 0.0f;
    
    battleLog.clear();
    std::string log = "Battle started!";
    if (currentMonster) log += " Encounter: " + currentMonster->getName();
    battleLog.push_back(log);
}

void BattleSystem::startBattle() {
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing!");
        return;
    }
    
    isBattleActive = true;
    battleLog.push_back("=== Battle Start ===");
}

void BattleSystem::endBattle() {
    isBattleActive = false;
    
    std::string resultStr;
    switch(lastResult) {
        case BattleResult::PLAYER_WIN: resultStr = "Player Wins"; break;
        case BattleResult::PLAYER_LOSE: resultStr = "Player Loses"; break;
        case BattleResult::PLAYER_FLEE: resultStr = "Player Fled"; break;
        case BattleResult::ONGOING: resultStr = "Battle Interrupted"; break;
    }
    
    battleLog.push_back("Battle ended: " + resultStr);
    battleLog.push_back("=== Battle End ===");
}

BattleResult BattleSystem::executeBattleRound() {
    if (!isBattleActive) {
        battleLog.push_back("Error: No active battle");
        return lastResult;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        isBattleActive = false;
        return lastResult;
    }
    
    if (!currentMonster->get_isAlive()) {
        battleLog.push_back("Monster defeated! Player wins!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_WIN;
        return lastResult;
    }
    
    if (!player->get_isAlive()) {
        battleLog.push_back("Player defeated!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_LOSE;
        return lastResult;
    }
    
    round_count++;
    battleLog.push_back("--- Round " + std::to_string(round_count) + " ---");
    
    std::uniform_int_distribution<int> actionDist(1, 100);
    int actionRoll = actionDist(gen);

    // Every round has tactical chances: defend / counter / normal attack.
    if (actionRoll <= 35) {
        playerDefend();
        monsterAttack();
    } else if (actionRoll <= 70) {
        bool counterSuccess = playerCounter();
        if (!counterSuccess && currentMonster && currentMonster->get_isAlive()) {
            // Failed counter: take 10% extra damage.
            monsterAttack(1.10f);
        }
    } else {
        playerAttack();
        if (currentMonster && currentMonster->get_isAlive()) {
            monsterAttack();
        }
    }
    return lastResult;
}

int BattleSystem::playerAttack() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return 0;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        return 0;
    }
    
    float playerAtk = player->get_ATK();
    std::uniform_real_distribution<float> atkDist(0.8f, 1.2f);
    float damageMultiplier = atkDist(gen);
    int baseDamage = static_cast<int>(playerAtk * damageMultiplier);
    
    int monsterDef = currentMonster->getDEF();
    int actualDamage = baseDamage - monsterDef;
    if (actualDamage < 1) actualDamage = 1;
    
    battleLog.push_back("Player attacks " + currentMonster->getName() + 
                       ", dealing " + std::to_string(actualDamage) + " damage");
    
    currentMonster->takeDamage(actualDamage);
    
    if (!currentMonster->get_isAlive()) {
        battleLog.push_back("Defeated " + currentMonster->getName() + "!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_WIN;
        
        float reward_exp = static_cast<float>(currentMonster->getExpReward());
        float reward_gold = static_cast<float>(currentMonster->getGoldReward());
        float reward_score = currentMonster->getScoreReward();
        
        reward[0] += reward_exp;
        reward[1] += reward_gold;
        reward[2] += reward_score;
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "Gained " << reward_exp << " EXP and " << reward_gold << " Gold";
        battleLog.push_back(ss.str());
    }
    battleLog.push_back("Monster remaining HP: " + std::to_string(currentMonster->getHP()));
    return actualDamage;
}

int BattleSystem::monsterAttack(float extraDamageMultiplier) {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return 0;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        return 0;
    }
    
    std::uniform_int_distribution<int> damageDist(
        static_cast<int>(currentMonster->getATK() * 0.8f),
        static_cast<int>(currentMonster->getATK() * 1.2f)
    );
    int baseDamage = damageDist(gen);

    float effectiveDef = player->get_DEF();
    if (playerDefending) {
        effectiveDef *= 1.8f;
    }

    int damage = baseDamage - static_cast<int>(effectiveDef);
    if (damage < 1) damage = 1;
    damage = static_cast<int>(damage * extraDamageMultiplier);
    if (damage < 1) damage = 1;
    player->change_HP(-static_cast<float>(damage));
    playerDefending = false;
    
    battleLog.push_back("Player remaining HP: " + std::to_string(static_cast<int>(player->get_HP())));
    
    if (damage > 0) {
        battleLog.push_back(currentMonster->getName() + " attacks player, dealing " + 
                           std::to_string(damage) + " damage");
        
        if (!player->get_isAlive()) {
            battleLog.push_back("Player defeated!");
            isBattleActive = false;
            lastResult = BattleResult::PLAYER_LOSE;
        }
    }
    
    return damage;
}

bool BattleSystem::playerFlee() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return false;
    }
    
    std::uniform_int_distribution<int> fleeChance(1, 100);
    int chance = fleeChance(gen);
    
    if (chance <= 70) {
        battleLog.push_back("Player successfully fled!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_FLEE;
        return true;
    } else {
        battleLog.push_back("Failed to flee!");
        return false;
    }
}

bool BattleSystem::playerDefend() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return false;
    }
    
    playerDefending = true;
    battleLog.push_back("Player chooses to defend (defense boosted this round)");
    return true;
}

bool BattleSystem::playerCounter() {
    if (!isBattleActive || !player || !currentMonster) return false;

    // Counter chance scales with ATK and enemy difficulty proxy.
    float atkFactor = player->get_ATK() / 250.0f;
    float difficultyPenalty = 0.10f;
    int enemyLevel = currentMonster->getLevel();
    if (enemyLevel >= 5) difficultyPenalty = 0.18f;
    else if (enemyLevel >= 3) difficultyPenalty = 0.14f;

    float successRate = 0.25f + atkFactor - difficultyPenalty;
    if (successRate < 0.08f) successRate = 0.08f;
    if (successRate > 0.65f) successRate = 0.65f;

    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    if (roll(gen) <= successRate) {
        battleLog.push_back("Counter success! No damage taken.");
        playerAttack();
        int bonusGold = std::max(1, static_cast<int>(currentMonster->getGoldReward() * 0.05f));
        reward[1] += static_cast<float>(bonusGold);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << static_cast<float>(bonusGold);
        battleLog.push_back("Counter bonus: +" + ss.str() + " gold.");
        return true;
    }

    battleLog.push_back("Counter failed! Incoming damage increased by 10%.");
    return false;
}

BattleResult BattleSystem::getLastResult() const { return lastResult; }
int BattleSystem::getRoundCount() const { return round_count; }
bool BattleSystem::get_isBattleActive() const { return isBattleActive; }
std::vector<std::string> BattleSystem::getBattleLog() const { return battleLog; }

const float* BattleSystem::getRewards() const {
    return reward;
}

std::string BattleSystem::showBattleLog() {
    std::stringstream ss;
    ss << "=== Battle Log ===" << std::endl;
    
    if (battleLog.empty()) {
        ss << "(No battle log)" << std::endl;
    } else {
        for (const auto& log : battleLog) {
            ss << log << std::endl;
        }
    }
    
    return ss.str();
}

void BattleSystem::applyRewards() {
    if (player) {
        player->change_EXP(reward[0]);
        player->change_Money(reward[1]);
        player->change_score(reward[2]);
    }
}
