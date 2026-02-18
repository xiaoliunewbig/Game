/*
 * 文件名: ConfigService.h
 * 说明: 配置管理服务，负责游戏配置的加载、解析和访问
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本类提供统一的配置管理接口，支持从文件加载配置，
 * 并提供类型安全的配置项访问方法。采用单例模式确保
 * 全局配置的一致性。
 */

#ifndef STRATEGY_CONFIG_SERVICE_H
#define STRATEGY_CONFIG_SERVICE_H

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <vector>

namespace strategy {

/**
 * @brief 配置管理服务
 * 
 * 提供统一的配置管理功能，支持从JSON/YAML文件加载配置，
 * 并提供类型安全的配置项访问接口。
 */
class ConfigService {
public:
    /**
     * @brief 获取单例实例
     * @return ConfigService实例引用
     */
    static ConfigService& GetInstance();

    /**
     * @brief 从文件加载配置
     * @param config_file 配置文件路径
     * @return 加载是否成功
     */
    bool LoadConfig(const std::string& config_file);

    /**
     * @brief 从JSON字符串加载配置
     * @param json_content JSON格式的配置内容
     * @return 加载是否成功
     */
    bool LoadFromJson(const std::string& json_content);

    /**
     * @brief 获取字符串配置项
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    std::string GetString(const std::string& key, const std::string& default_value = "");

    /**
     * @brief 获取整数配置项
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    int GetInt(const std::string& key, int default_value = 0);

    /**
     * @brief 获取布尔配置项
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    bool GetBool(const std::string& key, bool default_value = false);

    /**
     * @brief 获取浮点数配置项
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    double GetDouble(const std::string& key, double default_value = 0.0);

    /**
     * @brief 设置配置项
     * @param key 配置键
     * @param value 配置值
     */
    void SetString(const std::string& key, const std::string& value);

    /**
     * @brief 检查配置项是否存在
     * @param key 配置键
     * @return 是否存在
     */
    bool HasKey(const std::string& key) const;

    /**
     * @brief 重新加载配置
     * @return 重新加载是否成功
     */
    bool Reload();

    /**
     * @brief 获取所有配置键
     * @return 配置键列表
     */
    std::vector<std::string> GetAllKeys() const;

private:
    ConfigService() = default;
    ~ConfigService() = default;
    ConfigService(const ConfigService&) = delete;
    ConfigService& operator=(const ConfigService&) = delete;

    std::map<std::string, std::string> config_data_;  ///< 配置数据存储
    std::string config_file_path_;                    ///< 配置文件路径
    mutable std::mutex config_mutex_;                 ///< 线程安全锁

    /**
     * @brief 解析JSON配置内容
     * @param json_content JSON内容
     * @return 解析是否成功
     */
    bool ParseJsonContent(const std::string& json_content);

    /**
     * @brief 从嵌套键获取值
     * @param nested_key 嵌套键（如 "database.host"）
     * @return 配置值
     */
    std::string GetNestedValue(const std::string& nested_key) const;
};

} // namespace strategy

#endif // STRATEGY_CONFIG_SERVICE_H
