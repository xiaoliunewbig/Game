/*
 * 文件名: CooldownTracker.h
 * 说明: 技能冷却追踪系统（头文件 only）
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#ifndef ALGORITHM_COOLDOWN_TRACKER_H
#define ALGORITHM_COOLDOWN_TRACKER_H

#include <unordered_map>
#include <string>
#include <vector>

namespace algorithm {

/**
 * @brief 技能冷却追踪器
 * 管理每个角色每个技能的冷却状态
 */
class CooldownTracker {
public:
    CooldownTracker() = default;

    /**
     * @brief 启动冷却
     * @param character_id 角色ID
     * @param skill_id 技能ID
     * @param cooldown_ms 冷却时间（毫秒）
     */
    void startCooldown(int character_id, int skill_id, int cooldown_ms) {
        cooldowns_[character_id][skill_id] = cooldown_ms;
    }

    /**
     * @brief 检查技能是否可用（冷却完成）
     * @param character_id 角色ID
     * @param skill_id 技能ID
     * @return true 表示可用
     */
    bool isReady(int character_id, int skill_id) const {
        auto char_it = cooldowns_.find(character_id);
        if (char_it == cooldowns_.end()) return true;
        auto skill_it = char_it->second.find(skill_id);
        if (skill_it == char_it->second.end()) return true;
        return skill_it->second <= 0;
    }

    /**
     * @brief 获取剩余冷却时间
     * @param character_id 角色ID
     * @param skill_id 技能ID
     * @return 剩余毫秒数，0表示已就绪
     */
    int getRemainingCooldown(int character_id, int skill_id) const {
        auto char_it = cooldowns_.find(character_id);
        if (char_it == cooldowns_.end()) return 0;
        auto skill_it = char_it->second.find(skill_id);
        if (skill_it == char_it->second.end()) return 0;
        return skill_it->second > 0 ? skill_it->second : 0;
    }

    /**
     * @brief 更新所有冷却（每帧/每回合调用）
     * @param delta_ms 经过的时间（毫秒）
     */
    void tick(int delta_ms) {
        for (auto& [char_id, skills] : cooldowns_) {
            for (auto& [skill_id, remaining] : skills) {
                if (remaining > 0) {
                    remaining -= delta_ms;
                }
            }
        }
    }

    /**
     * @brief 重置某角色的所有冷却
     * @param character_id 角色ID
     */
    void resetCharacterCooldowns(int character_id) {
        cooldowns_.erase(character_id);
    }

    /**
     * @brief 重置所有冷却
     */
    void resetAll() {
        cooldowns_.clear();
    }

    /**
     * @brief 获取角色所有处于冷却中的技能ID
     * @param character_id 角色ID
     * @return 冷却中的技能ID列表
     */
    std::vector<int> getCooldownSkills(int character_id) const {
        std::vector<int> result;
        auto char_it = cooldowns_.find(character_id);
        if (char_it != cooldowns_.end()) {
            for (const auto& [skill_id, remaining] : char_it->second) {
                if (remaining > 0) {
                    result.push_back(skill_id);
                }
            }
        }
        return result;
    }

private:
    // character_id → (skill_id → remaining_ms)
    std::unordered_map<int, std::unordered_map<int, int>> cooldowns_;
};

} // namespace algorithm

#endif // ALGORITHM_COOLDOWN_TRACKER_H
