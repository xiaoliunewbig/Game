/*
 * 文件名: Player.h
 * 说明: 玩家角色类，管理玩家的所有数据和状态。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本类管理玩家角色的所有信息，包括基础属性、职业技能、
 * 装备状态、经验等级等，并提供数据持久化功能。
 */
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QMap>

/**
 * @brief 玩家职业枚举
 */
enum class PlayerProfession {
    Warrior,    ///< 战士
    Mage,       ///< 法师
    Archer      ///< 弓箭手
};

/**
 * @brief 玩家属性结构
 */
struct PlayerAttributes {
    int health = 100;           ///< 生命值
    int maxHealth = 100;        ///< 最大生命值
    int mana = 50;              ///< 魔法值
    int maxMana = 50;           ///< 最大魔法值
    int attack = 10;            ///< 攻击力
    int defense = 5;            ///< 防御力
    int speed = 10;             ///< 速度
    int luck = 5;               ///< 幸运值
};

/**
 * @brief 玩家状态效果
 */
struct StatusEffect {
    QString name;               ///< 状态名称
    int duration;               ///< 持续时间（秒）
    QJsonObject properties;     ///< 状态属性
};

/**
 * @brief 玩家角色类
 * 
 * 该类管理玩家角色的完整信息，包括：
 * - 基础信息（姓名、职业、等级等）
 * - 属性数据（生命、魔法、攻击等）
 * - 技能和装备状态
 * - 经验和成长系统
 * - 状态效果管理
 * - 数据持久化支持
 * 
 * 通过Q_PROPERTY暴露数据给QML，实现实时的UI数据绑定。
 */
class Player : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString profession READ getProfessionString NOTIFY professionChanged)
    Q_PROPERTY(int level READ getLevel NOTIFY levelChanged)
    Q_PROPERTY(int experience READ getExperience NOTIFY experienceChanged)
    Q_PROPERTY(int experienceToNext READ getExperienceToNext NOTIFY experienceChanged)
    Q_PROPERTY(int health READ getHealth NOTIFY healthChanged)
    Q_PROPERTY(int maxHealth READ getMaxHealth NOTIFY maxHealthChanged)
    Q_PROPERTY(int mana READ getMana NOTIFY manaChanged)
    Q_PROPERTY(int maxMana READ getMaxMana NOTIFY maxManaChanged)
    Q_PROPERTY(int attack READ getAttack NOTIFY attackChanged)
    Q_PROPERTY(int defense READ getDefense NOTIFY defenseChanged)
    Q_PROPERTY(int speed READ getSpeed NOTIFY speedChanged)
    Q_PROPERTY(int luck READ getLuck NOTIFY luckChanged)
    Q_PROPERTY(QJsonArray statusEffects READ getStatusEffectsJson NOTIFY statusEffectsChanged)

public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit Player(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~Player();

    // 基础信息访问器
    
    /**
     * @brief 获取玩家姓名
     * 
     * @return QString 玩家姓名
     */
    QString getName() const { return m_name; }
    
    /**
     * @brief 设置玩家姓名
     * 
     * @param name 新的玩家姓名
     */
    void setName(const QString &name);
    
    /**
     * @brief 获取玩家职业
     * 
     * @return PlayerProfession 玩家职业枚举
     */
    PlayerProfession getProfession() const { return m_profession; }
    
    /**
     * @brief 获取职业字符串
     * 
     * @return QString 职业的字符串表示
     */
    QString getProfessionString() const;
    
    /**
     * @brief 设置玩家职业
     * 
     * @param profession 新的职业
     */
    void setProfession(const QString &profession);
    
    /**
     * @brief 设置玩家职业
     * 
     * @param profession 职业枚举
     */
    void setProfession(PlayerProfession profession);
    
    /**
     * @brief 获取玩家等级
     * 
     * @return int 当前等级
     */
    int getLevel() const { return m_level; }
    
    /**
     * @brief 获取当前经验值
     * 
     * @return int 当前经验值
     */
    int getExperience() const { return m_experience; }
    
    /**
     * @brief 获取升级所需经验
     * 
     * @return int 升级所需的经验值
     */
    int getExperienceToNext() const;

    // 属性访问器
    
    /**
     * @brief 获取当前生命值
     * 
     * @return int 当前生命值
     */
    int getHealth() const { return m_attributes.health; }
    
    /**
     * @brief 获取最大生命值
     * 
     * @return int 最大生命值
     */
    int getMaxHealth() const { return m_attributes.maxHealth; }
    
    /**
     * @brief 获取当前魔法值
     * 
     * @return int 当前魔法值
     */
    int getMana() const { return m_attributes.mana; }
    
    /**
     * @brief 获取最大魔法值
     * 
     * @return int 最大魔法值
     */
    int getMaxMana() const { return m_attributes.maxMana; }
    
    /**
     * @brief 获取攻击力
     * 
     * @return int 攻击力
     */
    int getAttack() const { return m_attributes.attack; }
    
    /**
     * @brief 获取防御力
     * 
     * @return int 防御力
     */
    int getDefense() const { return m_attributes.defense; }
    
    /**
     * @brief 获取速度
     * 
     * @return int 速度
     */
    int getSpeed() const { return m_attributes.speed; }
    
    /**
     * @brief 获取幸运值
     * 
     * @return int 幸运值
     */
    int getLuck() const { return m_attributes.luck; }
    
    /**
     * @brief 获取完整属性结构
     * 
     * @return PlayerAttributes 玩家属性结构
     */
    PlayerAttributes getAttributes() const { return m_attributes; }

    // 游戏操作接口
    
    /**
     * @brief 初始化新玩家
     * 
     * 设置新玩家的初始属性和状态。
     */
    void initializeNewPlayer();
    
    /**
     * @brief 增加经验值
     * 
     * @param exp 要增加的经验值
     * @return bool 是否升级了
     */
    bool addExperience(int exp);
    
    /**
     * @brief 升级
     * 
     * 提升玩家等级并增加属性。
     */
    void levelUp();
    
    /**
     * @brief 受到伤害
     * 
     * @param damage 伤害值
     * @return bool 是否死亡
     */
    bool takeDamage(int damage);
    
    /**
     * @brief 恢复生命值
     * 
     * @param amount 恢复量
     */
    void heal(int amount);
    
    /**
     * @brief 消耗魔法值
     * 
     * @param amount 消耗量
     * @return bool 是否有足够的魔法值
     */
    bool consumeMana(int amount);
    
    /**
     * @brief 恢复魔法值
     * 
     * @param amount 恢复量
     */
    void restoreMana(int amount);
    
    /**
     * @brief 添加状态效果
     * 
     * @param effect 状态效果
     */
    void addStatusEffect(const StatusEffect &effect);
    
    /**
     * @brief 移除状态效果
     * 
     * @param effectName 状态效果名称
     */
    void removeStatusEffect(const QString &effectName);
    
    /**
     * @brief 检查是否有指定状态效果
     * 
     * @param effectName 状态效果名称
     * @return bool 是否存在该状态效果
     */
    bool hasStatusEffect(const QString &effectName) const;
    
    /**
     * @brief 获取状态效果JSON数组
     * 
     * @return QJsonArray 状态效果的JSON表示
     */
    QJsonArray getStatusEffectsJson() const;

    // QML可调用的方法
    
    /**
     * @brief 获取玩家信息JSON
     * 
     * @return QJsonObject 玩家信息的JSON表示
     */
    Q_INVOKABLE QJsonObject getPlayerInfo() const;
    
    /**
     * @brief 获取属性加成信息
     * 
     * @return QJsonObject 属性加成的详细信息
     */
    Q_INVOKABLE QJsonObject getAttributeBonus() const;
    
    /**
     * @brief 检查是否可以升级
     * 
     * @return bool 是否可以升级
     */
    Q_INVOKABLE bool canLevelUp() const;
    
    /**
     * @brief 获取升级预览
     * 
     * @return QJsonObject 升级后的属性预览
     */
    Q_INVOKABLE QJsonObject getLevelUpPreview() const;
    
    /**
     * @brief 重置属性点
     * 
     * 重置所有属性分配。
     */
    Q_INVOKABLE void resetAttributes();
    
    /**
     * @brief 获取职业技能列表
     * 
     * @return QJsonArray 职业技能的JSON数组
     */
    Q_INVOKABLE QJsonArray getProfessionSkills() const;

    // 数据持久化
    
    /**
     * @brief 转换为JSON对象
     * 
     * @return QJsonObject 玩家数据的JSON表示
     */
    QJsonObject toJson() const;
    
    /**
     * @brief 从JSON对象加载
     * 
     * @param json JSON对象
     * @return bool 是否加载成功
     */
    bool loadFromJson(const QJsonObject &json);
    
    /**
     * @brief 更新游戏逻辑
     * 
     * @param deltaTime 帧时间间隔
     */
    void update(float deltaTime);

signals:
    /**
     * @brief 姓名改变信号
     */
    void nameChanged();
    
    /**
     * @brief 职业改变信号
     */
    void professionChanged();
    
    /**
     * @brief 等级改变信号
     */
    void levelChanged();
    
    /**
     * @brief 经验改变信号
     */
    void experienceChanged();
    
    /**
     * @brief 生命值改变信号
     */
    void healthChanged();
    
    /**
     * @brief 最大生命值改变信号
     */
    void maxHealthChanged();
    
    /**
     * @brief 魔法值改变信号
     */
    void manaChanged();
    
    /**
     * @brief 最大魔法值改变信号
     */
    void maxManaChanged();
    
    /**
     * @brief 攻击力改变信号
     */
    void attackChanged();
    
    /**
     * @brief 防御力改变信号
     */
    void defenseChanged();
    
    /**
     * @brief 速度改变信号
     */
    void speedChanged();
    
    /**
     * @brief 幸运值改变信号
     */
    void luckChanged();
    
    /**
     * @brief 状态效果改变信号
     */
    void statusEffectsChanged();
    
    /**
     * @brief 升级信号
     * 
     * @param newLevel 新等级
     */
    void leveledUp(int newLevel);
    
    /**
     * @brief 死亡信号
     */
    void died();
    
    /**
     * @brief 状态效果添加信号
     * 
     * @param effectName 状态效果名称
     */
    void statusEffectAdded(const QString &effectName);
    
    /**
     * @brief 状态效果移除信号
     * 
     * @param effectName 状态效果名称
     */
    void statusEffectRemoved(const QString &effectName);

private:
    /**
     * @brief 计算等级所需经验
     * 
     * @param level 目标等级
     * @return int 所需经验值
     */
    int calculateExperienceForLevel(int level) const;
    
    /**
     * @brief 计算职业属性加成
     * 
     * @return PlayerAttributes 职业加成属性
     */
    PlayerAttributes calculateProfessionBonus() const;
    
    /**
     * @brief 更新状态效果
     * 
     * @param deltaTime 帧时间间隔
     */
    void updateStatusEffects(float deltaTime);
    
    /**
     * @brief 应用状态效果
     * 
     * 计算所有状态效果对属性的影响。
     */
    void applyStatusEffects();

private:
    /**
     * @brief 玩家姓名
     */
    QString m_name;
    
    /**
     * @brief 玩家职业
     */
    PlayerProfession m_profession;
    
    /**
     * @brief 玩家等级
     */
    int m_level;
    
    /**
     * @brief 当前经验值
     */
    int m_experience;
    
    /**
     * @brief 玩家基础属性
     */
    PlayerAttributes m_attributes;
    
    /**
     * @brief 基础属性（不包含加成）
     */
    PlayerAttributes m_baseAttributes;
    
    /**
     * @brief 当前状态效果列表
     */
    QMap<QString, StatusEffect> m_statusEffects;
    
    /**
     * @brief 职业技能点数
     */
    int m_skillPoints;
    
    /**
     * @brief 属性点数
     */
    int m_attributePoints;
    
    /**
     * @brief 玩家创建时间
     */
    qint64 m_createTime;
    
    /**
     * @brief 总游戏时间（秒）
     */
    int m_totalPlayTime;
};