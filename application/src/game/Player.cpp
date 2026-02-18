/*
 * 文件名: Player.cpp
 * 说明: 玩家角色类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本文件实现玩家角色类的所有方法，管理玩家的完整数据和状态。
 */
#include "game/Player.h"
#include <QDateTime>
#include <QDebug>
#include <QtMath>

Player::Player(QObject *parent)
    : QObject(parent)
    , m_name("未命名玩家")
    , m_profession(PlayerProfession::Warrior)
    , m_level(1)
    , m_experience(0)
    , m_skillPoints(0)
    , m_attributePoints(0)
    , m_totalPlayTime(0)
{
    // 设置创建时间
    m_createTime = QDateTime::currentMSecsSinceEpoch();
    
    // 初始化基础属性
    initializeNewPlayer();
    
    qDebug() << "Player: 玩家对象创建完成";
}

Player::~Player()
{
    qDebug() << "Player: 玩家对象销毁";
}

void Player::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
        qDebug() << "Player: 设置玩家姓名为" << name;
    }
}

QString Player::getProfessionString() const
{
    switch (m_profession) {
        case PlayerProfession::Warrior:
            return "战士";
        case PlayerProfession::Mage:
            return "法师";
        case PlayerProfession::Archer:
            return "弓箭手";
        default:
            return "未知";
    }
}

void Player::setProfession(const QString &profession)
{
    PlayerProfession newProfession = PlayerProfession::Warrior;
    
    if (profession == "战士" || profession == "Warrior") {
        newProfession = PlayerProfession::Warrior;
    } else if (profession == "法师" || profession == "Mage") {
        newProfession = PlayerProfession::Mage;
    } else if (profession == "弓箭手" || profession == "Archer") {
        newProfession = PlayerProfession::Archer;
    }
    
    setProfession(newProfession);
}

void Player::setProfession(PlayerProfession profession)
{
    if (m_profession != profession) {
        m_profession = profession;
        
        // 重新计算属性加成
        applyStatusEffects();
        
        emit professionChanged();
        qDebug() << "Player: 设置职业为" << getProfessionString();
    }
}

int Player::getExperienceToNext() const
{
    int nextLevelExp = calculateExperienceForLevel(m_level + 1);
    return nextLevelExp - m_experience;
}

void Player::initializeNewPlayer()
{
    // 重置等级和经验
    m_level = 1;
    m_experience = 0;
    m_skillPoints = 0;
    m_attributePoints = 5; // 初始属性点
    
    // 设置基础属性（根据职业）
    switch (m_profession) {
        case PlayerProfession::Warrior:
            m_baseAttributes = {120, 120, 30, 30, 15, 10, 8, 5};
            break;
        case PlayerProfession::Mage:
            m_baseAttributes = {80, 80, 100, 100, 8, 3, 12, 8};
            break;
        case PlayerProfession::Archer:
            m_baseAttributes = {100, 100, 50, 50, 12, 6, 15, 10};
            break;
    }
    
    // 复制到当前属性
    m_attributes = m_baseAttributes;
    
    // 清空状态效果
    m_statusEffects.clear();
    
    // 发送所有属性变更信号
    emit levelChanged();
    emit experienceChanged();
    emit healthChanged();
    emit maxHealthChanged();
    emit manaChanged();
    emit maxManaChanged();
    emit attackChanged();
    emit defenseChanged();
    emit speedChanged();
    emit luckChanged();
    emit statusEffectsChanged();
    
    qDebug() << "Player: 初始化新玩家完成 -" << getProfessionString();
}

bool Player::addExperience(int exp)
{
    if (exp <= 0) {
        return false;
    }
    
    m_experience += exp;
    emit experienceChanged();
    
    qDebug() << "Player: 获得经验" << exp << "总经验:" << m_experience;
    
    // 检查是否可以升级
    bool leveledUp = false;
    while (canLevelUp()) {
        levelUp();
        leveledUp = true;
    }
    
    return leveledUp;
}

void Player::levelUp()
{
    if (!canLevelUp()) {
        return;
    }
    
    m_level++;
    
    // 获得属性点和技能点
    m_attributePoints += 3;
    m_skillPoints += 1;
    
    // 职业特定的属性成长
    PlayerAttributes growth = calculateProfessionBonus();
    
    m_baseAttributes.maxHealth += growth.maxHealth;
    m_baseAttributes.maxMana += growth.maxMana;
    m_baseAttributes.attack += growth.attack;
    m_baseAttributes.defense += growth.defense;
    m_baseAttributes.speed += growth.speed;
    m_baseAttributes.luck += growth.luck;
    
    // 恢复生命值和魔法值
    m_attributes.health = m_baseAttributes.maxHealth;
    m_attributes.mana = m_baseAttributes.maxMana;
    
    // 重新应用所有效果
    applyStatusEffects();
    
    // 发送信号
    emit levelChanged();
    emit experienceChanged();
    emit healthChanged();
    emit maxHealthChanged();
    emit manaChanged();
    emit maxManaChanged();
    emit attackChanged();
    emit defenseChanged();
    emit speedChanged();
    emit luckChanged();
    emit leveledUp(m_level);
    
    qDebug() << "Player: 升级到" << m_level << "级！";
}

bool Player::takeDamage(int damage)
{
    if (damage <= 0) {
        return false;
    }
    
    // 计算实际伤害（考虑防御力）
    int actualDamage = qMax(1, damage - m_attributes.defense / 2);
    
    m_attributes.health = qMax(0, m_attributes.health - actualDamage);
    emit healthChanged();
    
    qDebug() << "Player: 受到伤害" << actualDamage << "剩余生命值:" << m_attributes.health;
    
    // 检查是否死亡
    if (m_attributes.health <= 0) {
        emit died();
        qDebug() << "Player: 玩家死亡";
        return true;
    }
    
    return false;
}

void Player::heal(int amount)
{
    if (amount <= 0) {
        return;
    }
    
    int oldHealth = m_attributes.health;
    m_attributes.health = qMin(m_attributes.maxHealth, m_attributes.health + amount);
    
    if (m_attributes.health != oldHealth) {
        emit healthChanged();
        qDebug() << "Player: 恢复生命值" << (m_attributes.health - oldHealth) 
                 << "当前生命值:" << m_attributes.health;
    }
}

bool Player::consumeMana(int amount)
{
    if (amount <= 0 || m_attributes.mana < amount) {
        return false;
    }
    
    m_attributes.mana -= amount;
    emit manaChanged();
    
    qDebug() << "Player: 消耗魔法值" << amount << "剩余魔法值:" << m_attributes.mana;
    return true;
}

void Player::restoreMana(int amount)
{
    if (amount <= 0) {
        return;
    }
    
    int oldMana = m_attributes.mana;
    m_attributes.mana = qMin(m_attributes.maxMana, m_attributes.mana + amount);
    
    if (m_attributes.mana != oldMana) {
        emit manaChanged();
        qDebug() << "Player: 恢复魔法值" << (m_attributes.mana - oldMana) 
                 << "当前魔法值:" << m_attributes.mana;
    }
}

void Player::addStatusEffect(const StatusEffect &effect)
{
    // 如果已存在同名效果，更新持续时间
    if (m_statusEffects.contains(effect.name)) {
        m_statusEffects[effect.name].duration = qMax(
            m_statusEffects[effect.name].duration, effect.duration);
    } else {
        m_statusEffects[effect.name] = effect;
        emit statusEffectAdded(effect.name);
    }
    
    // 重新应用状态效果
    applyStatusEffects();
    emit statusEffectsChanged();
    
    qDebug() << "Player: 添加状态效果" << effect.name << "持续时间:" << effect.duration;
}

void Player::removeStatusEffect(const QString &effectName)
{
    if (m_statusEffects.remove(effectName) > 0) {
        // 重新应用状态效果
        applyStatusEffects();
        emit statusEffectsChanged();
        emit statusEffectRemoved(effectName);
        
        qDebug() << "Player: 移除状态效果" << effectName;
    }
}

bool Player::hasStatusEffect(const QString &effectName) const
{
    return m_statusEffects.contains(effectName);
}

QJsonArray Player::getStatusEffectsJson() const
{
    QJsonArray array;
    
    for (auto it = m_statusEffects.begin(); it != m_statusEffects.end(); ++it) {
        QJsonObject effectObj;
        effectObj["name"] = it->name;
        effectObj["duration"] = it->duration;
        effectObj["properties"] = it->properties;
        array.append(effectObj);
    }
    
    return array;
}

QJsonObject Player::getPlayerInfo() const
{
    QJsonObject info;
    
    info["name"] = m_name;
    info["profession"] = getProfessionString();
    info["level"] = m_level;
    info["experience"] = m_experience;
    info["experienceToNext"] = getExperienceToNext();
    info["skillPoints"] = m_skillPoints;
    info["attributePoints"] = m_attributePoints;
    info["totalPlayTime"] = m_totalPlayTime;
    
    // 属性信息
    QJsonObject attributes;
    attributes["health"] = m_attributes.health;
    attributes["maxHealth"] = m_attributes.maxHealth;
    attributes["mana"] = m_attributes.mana;
    attributes["maxMana"] = m_attributes.maxMana;
    attributes["attack"] = m_attributes.attack;
    attributes["defense"] = m_attributes.defense;
    attributes["speed"] = m_attributes.speed;
    attributes["luck"] = m_attributes.luck;
    info["attributes"] = attributes;
    
    // 状态效果
    info["statusEffects"] = getStatusEffectsJson();
    
    return info;
}

QJsonObject Player::getAttributeBonus() const
{
    QJsonObject bonus;
    
    // 计算职业加成
    PlayerAttributes professionBonus = calculateProfessionBonus();
    
    QJsonObject professionObj;
    professionObj["health"] = professionBonus.health;
    professionObj["maxHealth"] = professionBonus.maxHealth;
    professionObj["mana"] = professionBonus.mana;
    professionObj["maxMana"] = professionBonus.maxMana;
    professionObj["attack"] = professionBonus.attack;
    professionObj["defense"] = professionBonus.defense;
    professionObj["speed"] = professionBonus.speed;
    professionObj["luck"] = professionBonus.luck;
    
    bonus["profession"] = professionObj;
    
    // 计算状态效果加成
    QJsonObject statusBonus;
    statusBonus["health"] = 0;
    statusBonus["maxHealth"] = 0;
    statusBonus["mana"] = 0;
    statusBonus["maxMana"] = 0;
    statusBonus["attack"] = 0;
    statusBonus["defense"] = 0;
    statusBonus["speed"] = 0;
    statusBonus["luck"] = 0;
    
    // 这里可以添加状态效果的具体计算逻辑
    
    bonus["statusEffects"] = statusBonus;
    
    return bonus;
}

bool Player::canLevelUp() const
{
    return m_experience >= calculateExperienceForLevel(m_level + 1);
}

QJsonObject Player::getLevelUpPreview() const
{
    if (!canLevelUp()) {
        return QJsonObject();
    }
    
    QJsonObject preview;
    
    // 预览等级
    preview["newLevel"] = m_level + 1;
    
    // 预览属性成长
    PlayerAttributes growth = calculateProfessionBonus();
    
    QJsonObject attributeGrowth;
    attributeGrowth["maxHealth"] = growth.maxHealth;
    attributeGrowth["maxMana"] = growth.maxMana;
    attributeGrowth["attack"] = growth.attack;
    attributeGrowth["defense"] = growth.defense;
    attributeGrowth["speed"] = growth.speed;
    attributeGrowth["luck"] = growth.luck;
    
    preview["attributeGrowth"] = attributeGrowth;
    
    // 预览获得的点数
    preview["skillPointsGained"] = 1;
    preview["attributePointsGained"] = 3;
    
    return preview;
}

void Player::resetAttributes()
{
    // 重置到基础属性
    m_attributes = m_baseAttributes;
    
    // 重新计算所有加成
    applyStatusEffects();
    
    // 发送所有属性变更信号
    emit healthChanged();
    emit maxHealthChanged();
    emit manaChanged();
    emit maxManaChanged();
    emit attackChanged();
    emit defenseChanged();
    emit speedChanged();
    emit luckChanged();
    
    qDebug() << "Player: 重置属性完成";
}

QJsonArray Player::getProfessionSkills() const
{
    QJsonArray skills;
    
    // 根据职业返回不同的技能列表
    switch (m_profession) {
        case PlayerProfession::Warrior:
            skills.append(QJsonObject{{"id", 1}, {"name", "重击"}, {"description", "造成150%攻击力的伤害"}});
            skills.append(QJsonObject{{"id", 2}, {"name", "防御姿态"}, {"description", "提高50%防御力，持续10秒"}});
            skills.append(QJsonObject{{"id", 3}, {"name", "战吼"}, {"description", "提高20%攻击力，持续30秒"}});
            break;
            
        case PlayerProfession::Mage:
            skills.append(QJsonObject{{"id", 11}, {"name", "火球术"}, {"description", "发射火球造成魔法伤害"}});
            skills.append(QJsonObject{{"id", 12}, {"name", "冰霜护甲"}, {"description", "减少受到的物理伤害"}});
            skills.append(QJsonObject{{"id", 13}, {"name", "魔法恢复"}, {"description", "快速恢复魔法值"}});
            break;
            
        case PlayerProfession::Archer:
            skills.append(QJsonObject{{"id", 21}, {"name", "精准射击"}, {"description", "必定命中并造成额外伤害"}});
            skills.append(QJsonObject{{"id", 22}, {"name", "多重射击"}, {"description", "同时射出3支箭"}});
            skills.append(QJsonObject{{"id", 23}, {"name", "闪避"}, {"description", "提高闪避率"}});
            break;
    }
    
    return skills;
}

QJsonObject Player::toJson() const
{
    QJsonObject json;
    
    json["name"] = m_name;
    json["profession"] = static_cast<int>(m_profession);
    json["level"] = m_level;
    json["experience"] = m_experience;
    json["skillPoints"] = m_skillPoints;
    json["attributePoints"] = m_attributePoints;
    json["createTime"] = m_createTime;
    json["totalPlayTime"] = m_totalPlayTime;
    
    // 基础属性
    QJsonObject baseAttrs;
    baseAttrs["health"] = m_baseAttributes.health;
    baseAttrs["maxHealth"] = m_baseAttributes.maxHealth;
    baseAttrs["mana"] = m_baseAttributes.mana;
    baseAttrs["maxMana"] = m_baseAttributes.maxMana;
    baseAttrs["attack"] = m_baseAttributes.attack;
    baseAttrs["defense"] = m_baseAttributes.defense;
    baseAttrs["speed"] = m_baseAttributes.speed;
    baseAttrs["luck"] = m_baseAttributes.luck;
    json["baseAttributes"] = baseAttrs;
    
    // 当前属性
    QJsonObject currentAttrs;
    currentAttrs["health"] = m_attributes.health;
    currentAttrs["maxHealth"] = m_attributes.maxHealth;
    currentAttrs["mana"] = m_attributes.mana;
    currentAttrs["maxMana"] = m_attributes.maxMana;
    currentAttrs["attack"] = m_attributes.attack;
    currentAttrs["defense"] = m_attributes.defense;
    currentAttrs["speed"] = m_attributes.speed;
    currentAttrs["luck"] = m_attributes.luck;
    json["attributes"] = currentAttrs;
    
    // 状态效果
    json["statusEffects"] = getStatusEffectsJson();
    
    return json;
}

bool Player::loadFromJson(const QJsonObject &json)
{
    try {
        m_name = json["name"].toString("未命名玩家");
        m_profession = static_cast<PlayerProfession>(json["profession"].toInt(0));
        m_level = json["level"].toInt(1);
        m_experience = json["experience"].toInt(0);
        m_skillPoints = json["skillPoints"].toInt(0);
        m_attributePoints = json["attributePoints"].toInt(0);
        m_createTime = json["createTime"].toVariant().toLongLong();
        m_totalPlayTime = json["totalPlayTime"].toInt(0);
        
        // 加载基础属性
        QJsonObject baseAttrs = json["baseAttributes"].toObject();
        m_baseAttributes.health = baseAttrs["health"].toInt(100);
        m_baseAttributes.maxHealth = baseAttrs["maxHealth"].toInt(100);
        m_baseAttributes.mana = baseAttrs["mana"].toInt(50);
        m_baseAttributes.maxMana = baseAttrs["maxMana"].toInt(50);
        m_baseAttributes.attack = baseAttrs["attack"].toInt(10);
        m_baseAttributes.defense = baseAttrs["defense"].toInt(5);
        m_baseAttributes.speed = baseAttrs["speed"].toInt(10);
        m_baseAttributes.luck = baseAttrs["luck"].toInt(5);
        
        // 加载当前属性
        QJsonObject currentAttrs = json["attributes"].toObject();
        m_attributes.health = currentAttrs["health"].toInt(m_baseAttributes.health);
        m_attributes.maxHealth = currentAttrs["maxHealth"].toInt(m_baseAttributes.maxHealth);
        m_attributes.mana = currentAttrs["mana"].toInt(m_baseAttributes.mana);
        m_attributes.maxMana = currentAttrs["maxMana"].toInt(m_baseAttributes.maxMana);
        m_attributes.attack = currentAttrs["attack"].toInt(m_baseAttributes.attack);
        m_attributes.defense = currentAttrs["defense"].toInt(m_baseAttributes.defense);
        m_attributes.speed = currentAttrs["speed"].toInt(m_baseAttributes.speed);
        m_attributes.luck = currentAttrs["luck"].toInt(m_baseAttributes.luck);
        
        // 加载状态效果
        m_statusEffects.clear();
        QJsonArray statusArray = json["statusEffects"].toArray();
        for (const QJsonValue &value : statusArray) {
            QJsonObject effectObj = value.toObject();
            StatusEffect effect;
            effect.name = effectObj["name"].toString();
            effect.duration = effectObj["duration"].toInt();
            effect.properties = effectObj["properties"].toObject();
            m_statusEffects[effect.name] = effect;
        }
        
        // 发送所有变更信号
        emit nameChanged();
        emit professionChanged();
        emit levelChanged();
        emit experienceChanged();
        emit healthChanged();
        emit maxHealthChanged();
        emit manaChanged();
        emit maxManaChanged();
        emit attackChanged();
        emit defenseChanged();
        emit speedChanged();
        emit luckChanged();
        emit statusEffectsChanged();
        
        qDebug() << "Player: 从JSON加载玩家数据成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Player: 从JSON加载失败:" << e.what();
        return false;
    }
}

void Player::update(float deltaTime)
{
    // 更新游戏时间
    m_totalPlayTime += static_cast<int>(deltaTime);
    
    // 更新状态效果
    updateStatusEffects(deltaTime);
}

int Player::calculateExperienceForLevel(int level) const
{
    // 使用指数增长公式：exp = 100 * level^1.5
    return static_cast<int>(100 * qPow(level, 1.5));
}

PlayerAttributes Player::calculateProfessionBonus() const
{
    PlayerAttributes bonus = {0, 0, 0, 0, 0, 0, 0, 0};
    
    // 每级的属性成长（根据职业不同）
    switch (m_profession) {
        case PlayerProfession::Warrior:
            bonus.maxHealth = 15;
            bonus.maxMana = 3;
            bonus.attack = 3;
            bonus.defense = 2;
            bonus.speed = 1;
            bonus.luck = 1;
            break;
            
        case PlayerProfession::Mage:
            bonus.maxHealth = 8;
            bonus.maxMana = 12;
            bonus.attack = 1;
            bonus.defense = 1;
            bonus.speed = 2;
            bonus.luck = 2;
            break;
            
        case PlayerProfession::Archer:
            bonus.maxHealth = 12;
            bonus.maxMana = 6;
            bonus.attack = 2;
            bonus.defense = 1;
            bonus.speed = 3;
            bonus.luck = 2;
            break;
    }
    
    return bonus;
}

void Player::updateStatusEffects(float deltaTime)
{
    QStringList expiredEffects;
    
    // 更新所有状态效果的持续时间
    for (auto it = m_statusEffects.begin(); it != m_statusEffects.end(); ++it) {
        it->duration -= static_cast<int>(deltaTime * 1000); // 转换为毫秒
        
        if (it->duration <= 0) {
            expiredEffects.append(it.key());
        }
    }
    
    // 移除过期的状态效果
    for (const QString &effectName : expiredEffects) {
        removeStatusEffect(effectName);
    }
}

void Player::applyStatusEffects()
{
    // 重置到基础属性
    m_attributes = m_baseAttributes;
    
    // 应用职业加成
    PlayerAttributes professionBonus = calculateProfessionBonus();
    int levelBonus = m_level - 1; // 1级时没有加成
    
    m_attributes.maxHealth += professionBonus.maxHealth * levelBonus;
    m_attributes.maxMana += professionBonus.maxMana * levelBonus;
    m_attributes.attack += professionBonus.attack * levelBonus;
    m_attributes.defense += professionBonus.defense * levelBonus;
    m_attributes.speed += professionBonus.speed * levelBonus;
    m_attributes.luck += professionBonus.luck * levelBonus;
    
    // 应用状态效果
    for (const StatusEffect &effect : m_statusEffects) {
        QJsonObject props = effect.properties;
        
        if (props.contains("healthBonus")) {
            m_attributes.health += props["healthBonus"].toInt();
        }
        if (props.contains("maxHealthBonus")) {
            m_attributes.maxHealth += props["maxHealthBonus"].toInt();
        }
        if (props.contains("manaBonus")) {
            m_attributes.mana += props["manaBonus"].toInt();
        }
        if (props.contains("maxManaBonus")) {
            m_attributes.maxMana += props["maxManaBonus"].toInt();
        }
        if (props.contains("attackBonus")) {
            m_attributes.attack += props["attackBonus"].toInt();
        }
        if (props.contains("defenseBonus")) {
            m_attributes.defense += props["defenseBonus"].toInt();
        }
        if (props.contains("speedBonus")) {
            m_attributes.speed += props["speedBonus"].toInt();
        }
        if (props.contains("luckBonus")) {
            m_attributes.luck += props["luckBonus"].toInt();
        }
    }
    
    // 确保当前生命值和魔法值不超过最大值
    m_attributes.health = qMin(m_attributes.health, m_attributes.maxHealth);
    m_attributes.mana = qMin(m_attributes.mana, m_attributes.maxMana);
}