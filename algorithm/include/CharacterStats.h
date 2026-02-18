/*
 * 文件名: CharacterStats.h
 * 说明: 角色属性系统定义，包括职业、元素、伤害类型和属性结构
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#ifndef ALGORITHM_CHARACTER_STATS_H
#define ALGORITHM_CHARACTER_STATS_H

#include <string>
#include <unordered_map>
#include <vector>
#include <array>

namespace algorithm {

/**
 * @brief 职业枚举
 */
enum class Profession {
    None = 0,
    Warrior = 1,   // 战士
    Mage = 2,      // 法师
    Archer = 3     // 弓箭手
};

/**
 * @brief 元素类型枚举
 */
enum class Element {
    None = 0,
    Fire = 1,      // 火
    Water = 2,     // 水
    Grass = 3,     // 草
    Light = 4,     // 光
    Dark = 5       // 暗
};

constexpr int ELEMENT_COUNT = 6;

/**
 * @brief 伤害类型枚举
 */
enum class DamageType {
    Physical = 0,  // 物理伤害
    Magical = 1,   // 魔法伤害
    Pure = 2       // 纯粹伤害（无视防御）
};

/**
 * @brief 角色属性结构体
 */
struct CharacterStats {
    int character_id = 0;          ///< 角色ID
    std::string name;              ///< 角色名称
    Profession profession = Profession::None;  ///< 职业
    Element element = Element::None;           ///< 元素属性
    int level = 1;                 ///< 等级

    // 基础属性
    float max_hp = 100.0f;         ///< 最大生命值
    float current_hp = 100.0f;     ///< 当前生命值
    float max_mp = 50.0f;          ///< 最大魔法值
    float current_mp = 50.0f;      ///< 当前魔法值
    float attack = 10.0f;          ///< 物理攻击力
    float defense = 5.0f;          ///< 物理防御力
    float magic_attack = 10.0f;    ///< 魔法攻击力
    float magic_defense = 5.0f;    ///< 魔法防御力
    float speed = 10.0f;           ///< 速度
    float luck = 5.0f;             ///< 幸运值

    // 战斗属性
    float crit_rate = 0.05f;       ///< 暴击率 (0.0 - 1.0)
    float crit_damage = 0.5f;      ///< 暴击伤害加成 (1.0 + crit_damage = 暴击倍率)

    /**
     * @brief 获取HP百分比 (0-100)
     */
    float getHpPercent() const {
        return max_hp > 0 ? (current_hp / max_hp) * 100.0f : 0.0f;
    }

    /**
     * @brief 获取MP百分比 (0-100)
     */
    float getMpPercent() const {
        return max_mp > 0 ? (current_mp / max_mp) * 100.0f : 0.0f;
    }
};

/**
 * @brief 角色属性注册表
 * 管理默认属性模板和元素克制矩阵
 */
class CharacterStatsRegistry {
public:
    CharacterStatsRegistry();

    /**
     * @brief 获取元素克制倍率
     * @param attacker_element 攻击者元素
     * @param defender_element 防御者元素
     * @return 伤害倍率
     */
    float getElementMultiplier(Element attacker_element, Element defender_element) const;

    /**
     * @brief 获取职业默认属性
     * @param profession 职业类型
     * @param level 等级
     * @return 默认属性
     */
    CharacterStats getDefaultStats(Profession profession, int level = 1) const;

    /**
     * @brief 注册/更新角色属性
     * @param stats 角色属性
     */
    void registerCharacter(const CharacterStats& stats);

    /**
     * @brief 查询角色属性
     * @param character_id 角色ID
     * @return 角色属性指针，不存在则返回nullptr
     */
    const CharacterStats* getCharacterStats(int character_id) const;

    /**
     * @brief 根据等级缩放属性
     * @param base 基础属性
     * @param level 目标等级
     * @return 缩放后的属性
     */
    static CharacterStats scaleByLevel(const CharacterStats& base, int level);

private:
    // 元素克制矩阵 [攻击者元素][防御者元素]
    std::array<std::array<float, ELEMENT_COUNT>, ELEMENT_COUNT> element_matrix_;

    // 职业默认属性模板 (1级)
    std::unordered_map<int, CharacterStats> profession_defaults_;

    // 已注册角色
    std::unordered_map<int, CharacterStats> characters_;

    void initializeElementMatrix();
    void initializeProfessionDefaults();
};

} // namespace algorithm

#endif // ALGORITHM_CHARACTER_STATS_H
