/*
 * 文件名: CharacterStats.cpp
 * 说明: 角色属性系统实现，包括元素克制矩阵和职业默认属性
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#include "CharacterStats.h"
#include <cmath>
#include <algorithm>

namespace algorithm {

CharacterStatsRegistry::CharacterStatsRegistry() {
    initializeElementMatrix();
    initializeProfessionDefaults();
}

float CharacterStatsRegistry::getElementMultiplier(Element attacker_element, Element defender_element) const {
    int atk = static_cast<int>(attacker_element);
    int def = static_cast<int>(defender_element);
    if (atk >= 0 && atk < ELEMENT_COUNT && def >= 0 && def < ELEMENT_COUNT) {
        return element_matrix_[atk][def];
    }
    return 1.0f;
}

CharacterStats CharacterStatsRegistry::getDefaultStats(Profession profession, int level) const {
    int key = static_cast<int>(profession);
    auto it = profession_defaults_.find(key);
    if (it != profession_defaults_.end()) {
        return scaleByLevel(it->second, level);
    }
    // 无职业默认
    CharacterStats base;
    base.profession = profession;
    base.level = level;
    return scaleByLevel(base, level);
}

void CharacterStatsRegistry::registerCharacter(const CharacterStats& stats) {
    characters_[stats.character_id] = stats;
}

const CharacterStats* CharacterStatsRegistry::getCharacterStats(int character_id) const {
    auto it = characters_.find(character_id);
    return it != characters_.end() ? &it->second : nullptr;
}

CharacterStats CharacterStatsRegistry::scaleByLevel(const CharacterStats& base, int level) {
    if (level <= 1) return base;

    CharacterStats scaled = base;
    scaled.level = level;
    float factor = 1.0f + (level - 1) * 0.12f;  // 每级+12%

    scaled.max_hp = base.max_hp * factor;
    scaled.current_hp = scaled.max_hp;
    scaled.max_mp = base.max_mp * (1.0f + (level - 1) * 0.08f);  // MP每级+8%
    scaled.current_mp = scaled.max_mp;
    scaled.attack = base.attack * factor;
    scaled.defense = base.defense * factor;
    scaled.magic_attack = base.magic_attack * factor;
    scaled.magic_defense = base.magic_defense * factor;
    scaled.speed = base.speed * (1.0f + (level - 1) * 0.05f);  // 速度每级+5%
    // luck, crit_rate, crit_damage 不随等级变化

    return scaled;
}

void CharacterStatsRegistry::initializeElementMatrix() {
    // 默认所有倍率为 1.0
    for (auto& row : element_matrix_) {
        row.fill(1.0f);
    }

    int fire  = static_cast<int>(Element::Fire);
    int water = static_cast<int>(Element::Water);
    int grass = static_cast<int>(Element::Grass);
    int light = static_cast<int>(Element::Light);
    int dark  = static_cast<int>(Element::Dark);

    // 火 → 水: 0.5x (不利), 水 → 火: 2.0x (克制)
    element_matrix_[fire][water]  = 0.5f;
    element_matrix_[water][fire]  = 2.0f;

    // 火 → 草: 2.0x (克制), 草 → 火: 0.5x (不利)
    element_matrix_[fire][grass]  = 2.0f;
    element_matrix_[grass][fire]  = 0.5f;

    // 水 → 草: 0.5x (不利), 草 → 水: 2.0x (克制)
    element_matrix_[water][grass] = 0.5f;
    element_matrix_[grass][water] = 2.0f;

    // 光 ↔ 暗: 互相克制 2.0x
    element_matrix_[light][dark]  = 2.0f;
    element_matrix_[dark][light]  = 2.0f;

    // 同元素: 0.75x
    element_matrix_[fire][fire]   = 0.75f;
    element_matrix_[water][water] = 0.75f;
    element_matrix_[grass][grass] = 0.75f;
    element_matrix_[light][light] = 0.75f;
    element_matrix_[dark][dark]   = 0.75f;
}

void CharacterStatsRegistry::initializeProfessionDefaults() {
    // 战士：高HP、高物攻、高物防，低魔攻
    CharacterStats warrior;
    warrior.profession = Profession::Warrior;
    warrior.max_hp = 200.0f;
    warrior.current_hp = 200.0f;
    warrior.max_mp = 30.0f;
    warrior.current_mp = 30.0f;
    warrior.attack = 25.0f;
    warrior.defense = 20.0f;
    warrior.magic_attack = 5.0f;
    warrior.magic_defense = 8.0f;
    warrior.speed = 8.0f;
    warrior.luck = 5.0f;
    warrior.crit_rate = 0.08f;
    warrior.crit_damage = 0.5f;
    profession_defaults_[static_cast<int>(Profession::Warrior)] = warrior;

    // 法师：高魔攻、高MP，低HP、低物防
    CharacterStats mage;
    mage.profession = Profession::Mage;
    mage.max_hp = 120.0f;
    mage.current_hp = 120.0f;
    mage.max_mp = 100.0f;
    mage.current_mp = 100.0f;
    mage.attack = 5.0f;
    mage.defense = 6.0f;
    mage.magic_attack = 28.0f;
    mage.magic_defense = 18.0f;
    mage.speed = 10.0f;
    mage.luck = 8.0f;
    mage.crit_rate = 0.06f;
    mage.crit_damage = 0.7f;
    profession_defaults_[static_cast<int>(Profession::Mage)] = mage;

    // 弓箭手：高速度、高暴击，平衡攻防
    CharacterStats archer;
    archer.profession = Profession::Archer;
    archer.max_hp = 150.0f;
    archer.current_hp = 150.0f;
    archer.max_mp = 50.0f;
    archer.current_mp = 50.0f;
    archer.attack = 20.0f;
    archer.defense = 10.0f;
    archer.magic_attack = 12.0f;
    archer.magic_defense = 10.0f;
    archer.speed = 18.0f;
    archer.luck = 15.0f;
    archer.crit_rate = 0.12f;
    archer.crit_damage = 0.6f;
    profession_defaults_[static_cast<int>(Profession::Archer)] = archer;
}

} // namespace algorithm
