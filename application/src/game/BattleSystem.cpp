/*
 * 文件名: BattleSystem.cpp
 * 说明: 战斗系统的具体实现。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本文件实现战斗系统的所有功能，包括回合管理、伤害计算、
 * 技能释放和战斗结果处理。
 */
#include "game/BattleSystem.h"
#include "game/Player.h"
#include "network/AlgorithmServiceClient.h"
#include <QTimer>
#include <QDebug>
#include <QRandomGenerator>

BattleSystem::BattleSystem(QObject *parent)
    : QObject(parent)
    , m_player(nullptr)
    , m_algorithmClient(nullptr)
    , m_battleState(BattleState::Idle)
    , m_currentTurn(0)
    , m_turnTimer(nullptr)
    , m_turnDuration(30000) // 30秒回合时间
    , m_battleSpeed(1.0f)
    , m_autoMode(false)
{
    // 初始化回合定时器
    m_turnTimer = new QTimer(this);
    m_turnTimer->setSingleShot(true);
    connect(m_turnTimer, &QTimer::timeout, this, &BattleSystem::onTurnTimeout);
    
    qDebug() << "BattleSystem: 战斗系统初始化完成";
}

BattleSystem::~BattleSystem()
{
    endBattle();
}

bool BattleSystem::initialize(Player *player, AlgorithmServiceClient *algorithmClient)
{
    if (!player || !algorithmClient) {
        qCritical() << "BattleSystem: 初始化参数无效";
        return false;
    }
    
    m_player = player;
    m_algorithmClient = algorithmClient;
    
    // 连接玩家信号
    connect(m_player, &Player::died, this, &BattleSystem::onPlayerDied);
    
    qDebug() << "BattleSystem: 初始化成功";
    return true;
}

bool BattleSystem::startBattle(const QJsonObject &battleConfig)
{
    if (m_battleState != BattleState::Idle) {
        qWarning() << "BattleSystem: 战斗已在进行中";
        return false;
    }
    
    qDebug() << "BattleSystem: 开始战斗";
    
    // 解析战斗配置
    if (!parseBattleConfig(battleConfig)) {
        qCritical() << "BattleSystem: 战斗配置解析失败";
        return false;
    }
    
    // 初始化战斗状态
    m_battleState = BattleState::Starting;
    m_currentTurn = 0;
    m_battleLog.clear();
    
    // 计算行动顺序
    calculateTurnOrder();
    
    // 开始第一回合
    m_battleState = BattleState::PlayerTurn;
    startPlayerTurn();
    
    emit battleStarted();
    addBattleLog("战斗开始！");
    
    return true;
}

void BattleSystem::endBattle()
{
    if (m_battleState == BattleState::Idle) {
        return;
    }
    
    // 停止定时器
    m_turnTimer->stop();
    
    // 重置状态
    m_battleState = BattleState::Idle;
    m_enemies.clear();
    m_turnOrder.clear();
    m_currentTurn = 0;
    
    emit battleEnded();
    qDebug() << "BattleSystem: 战斗结束";
}

bool BattleSystem::playerAttack(int targetId, int skillId)
{
    if (m_battleState != BattleState::PlayerTurn) {
        qWarning() << "BattleSystem: 不是玩家回合";
        return false;
    }
    
    // 查找目标敌人
    auto targetIt = std::find_if(m_enemies.begin(), m_enemies.end(),
        [targetId](const Enemy &enemy) { return enemy.id == targetId; });
    
    if (targetIt == m_enemies.end()) {
        qWarning() << "BattleSystem: 目标敌人不存在" << targetId;
        return false;
    }
    
    if (targetIt->health <= 0) {
        qWarning() << "BattleSystem: 目标敌人已死亡";
        return false;
    }
    
    // 执行攻击
    executePlayerAttack(*targetIt, skillId);
    
    return true;
}

bool BattleSystem::playerUseItem(int itemId, int targetId)
{
    if (m_battleState != BattleState::PlayerTurn) {
        qWarning() << "BattleSystem: 不是玩家回合";
        return false;
    }
    
    // 这里应该检查玩家背包中是否有该物品
    // 暂时简化处理
    
    executePlayerItemUse(itemId, targetId);
    return true;
}

bool BattleSystem::playerDefend()
{
    if (m_battleState != BattleState::PlayerTurn) {
        qWarning() << "BattleSystem: 不是玩家回合";
        return false;
    }
    
    // 防御状态：减少50%受到的伤害
    StatusEffect defenseEffect;
    defenseEffect.name = "防御";
    defenseEffect.duration = 1; // 持续到下回合
    defenseEffect.properties["defenseBonus"] = m_player->getDefense() / 2;
    
    m_player->addStatusEffect(defenseEffect);
    
    addBattleLog(QString("%1 进入防御状态").arg(m_player->getName()));
    
    // 结束玩家回合
    endPlayerTurn();
    
    return true;
}

void BattleSystem::skipTurn()
{
    if (m_battleState == BattleState::PlayerTurn) {
        addBattleLog(QString("%1 跳过了这个回合").arg(m_player->getName()));
        endPlayerTurn();
    }
}

void BattleSystem::setBattleSpeed(float speed)
{
    m_battleSpeed = qBound(0.5f, speed, 3.0f);
    emit battleSpeedChanged(m_battleSpeed);
    
    qDebug() << "BattleSystem: 设置战斗速度为" << m_battleSpeed;
}

float BattleSystem::getBattleSpeed() const
{
    return m_battleSpeed;
}

void BattleSystem::setAutoMode(bool enabled)
{
    if (m_autoMode != enabled) {
        m_autoMode = enabled;
        emit autoModeChanged(enabled);
        
        qDebug() << "BattleSystem: 自动战斗模式" << (enabled ? "开启" : "关闭");
        
        // 如果在玩家回合开启自动模式，立即执行自动行动
        if (enabled && m_battleState == BattleState::PlayerTurn) {
            QTimer::singleShot(1000, this, &BattleSystem::executeAutoAction);
        }
    }
}

bool BattleSystem::isAutoMode() const
{
    return m_autoMode;
}

BattleState BattleSystem::getBattleState() const
{
    return m_battleState;
}

QJsonArray BattleSystem::getEnemies() const
{
    QJsonArray array;
    
    for (const Enemy &enemy : m_enemies) {
        QJsonObject enemyObj;
        enemyObj["id"] = enemy.id;
        enemyObj["name"] = enemy.name;
        enemyObj["health"] = enemy.health;
        enemyObj["maxHealth"] = enemy.maxHealth;
        enemyObj["attack"] = enemy.attack;
        enemyObj["defense"] = enemy.defense;
        enemyObj["speed"] = enemy.speed;
        enemyObj["level"] = enemy.level;
        array.append(enemyObj);
    }
    
    return array;
}

QStringList BattleSystem::getBattleLog() const
{
    return m_battleLog;
}

int BattleSystem::getCurrentTurn() const
{
    return m_currentTurn;
}

QJsonArray BattleSystem::getTurnOrder() const
{
    QJsonArray array;
    
    for (int entityId : m_turnOrder) {
        QJsonObject turnObj;
        if (entityId == -1) { // 玩家
            turnObj["type"] = "player";
            turnObj["name"] = m_player->getName();
            turnObj["id"] = -1;
        } else {
            // 查找敌人
            auto it = std::find_if(m_enemies.begin(), m_enemies.end(),
                [entityId](const Enemy &enemy) { return enemy.id == entityId; });
            if (it != m_enemies.end()) {
                turnObj["type"] = "enemy";
                turnObj["name"] = it->name;
                turnObj["id"] = it->id;
            }
        }
        array.append(turnObj);
    }
    
    return array;
}

void BattleSystem::update(float deltaTime)
{
    Q_UNUSED(deltaTime);
    // 更新战斗逻辑
    if (m_battleState == BattleState::Idle) {
        return;
    }
    
    // 这里可以添加战斗动画、特效等更新逻辑
    // 暂时简化处理
}

bool BattleSystem::parseBattleConfig(const QJsonObject &config)
{
    try {
        // 解析敌人列表
        QJsonArray enemiesArray = config["enemies"].toArray();
        m_enemies.clear();
        
        for (int i = 0; i < enemiesArray.size(); ++i) {
            QJsonObject enemyObj = enemiesArray[i].toObject();
            
            Enemy enemy;
            enemy.id = i + 1;
            enemy.name = enemyObj["name"].toString("未知敌人");
            enemy.maxHealth = enemyObj["health"].toInt(100);
            enemy.health = enemy.maxHealth;
            enemy.attack = enemyObj["attack"].toInt(10);
            enemy.defense = enemyObj["defense"].toInt(5);
            enemy.speed = enemyObj["speed"].toInt(10);
            enemy.level = enemyObj["level"].toInt(1);
            
            m_enemies.append(enemy);
        }
        
        // 解析其他配置
        m_turnDuration = config["turnDuration"].toInt(30000);
        
        qDebug() << "BattleSystem: 战斗配置解析完成，敌人数量:" << m_enemies.size();
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "BattleSystem: 解析战斗配置失败:" << e.what();
        return false;
    }
}

void BattleSystem::calculateTurnOrder()
{
    m_turnOrder.clear();
    
    // 收集所有参战单位的速度
    QList<QPair<int, int>> speedList; // <entityId, speed>
    
    // 添加玩家（ID为-1）
    speedList.append({-1, m_player->getSpeed()});
    
    // 添加敌人
    for (const Enemy &enemy : m_enemies) {
        if (enemy.health > 0) {
            speedList.append({enemy.id, enemy.speed});
        }
    }
    
    // 按速度排序（速度高的先行动）
    std::sort(speedList.begin(), speedList.end(),
        [](const QPair<int, int> &a, const QPair<int, int> &b) {
            return a.second > b.second;
        });
    
    // 构建回合顺序
    for (const auto &pair : speedList) {
        m_turnOrder.append(pair.first);
    }
    
    qDebug() << "BattleSystem: 回合顺序计算完成";
}

void BattleSystem::startPlayerTurn()
{
    m_battleState = BattleState::PlayerTurn;
    
    // 启动回合计时器
    int adjustedDuration = static_cast<int>(m_turnDuration / m_battleSpeed);
    m_turnTimer->start(adjustedDuration);
    
    emit playerTurnStarted();
    addBattleLog(QString("轮到 %1 行动").arg(m_player->getName()));
    
    // 如果是自动模式，延迟执行自动行动
    if (m_autoMode) {
        QTimer::singleShot(1000, this, &BattleSystem::executeAutoAction);
    }
}

void BattleSystem::endPlayerTurn()
{
    m_turnTimer->stop();
    
    emit playerTurnEnded();
    
    // 检查战斗是否结束
    if (checkBattleEnd()) {
        return;
    }
    
    // 进入敌人回合
    startEnemyTurn();
}

void BattleSystem::startEnemyTurn()
{
    m_battleState = BattleState::EnemyTurn;
    
    emit enemyTurnStarted();
    
    // 执行所有存活敌人的行动
    executeEnemyActions();
}

void BattleSystem::endEnemyTurn()
{
    emit enemyTurnEnded();
    
    // 检查战斗是否结束
    if (checkBattleEnd()) {
        return;
    }
    
    // 增加回合数
    m_currentTurn++;
    emit turnChanged(m_currentTurn);
    
    // 开始下一个玩家回合
    startPlayerTurn();
}

void BattleSystem::executePlayerAttack(Enemy &target, int skillId)
{
    // 调用算法服务计算伤害
    if (m_algorithmClient) {
        // 构建伤害计算请求
        QJsonObject request;
        request["attackerId"] = -1; // 玩家ID
        request["defenderId"] = target.id;
        request["skillId"] = skillId;
        request["attackerLevel"] = m_player->getLevel();
        request["attackerAttack"] = m_player->getAttack();
        request["defenderDefense"] = target.defense;
        
        // 异步调用算法服务
        m_algorithmClient->calculateDamage(request, [this, &target, skillId](const QJsonObject &result) {
            int damage = result["damage"].toInt(0);
            bool isCritical = result["isCritical"].toBool(false);
            QString description = result["description"].toString();
            
            // 应用伤害
            target.health = qMax(0, target.health - damage);
            
            // 添加战斗日志
            QString logMessage = QString("%1 对 %2 使用技能造成了 %3 点伤害")
                .arg(m_player->getName())
                .arg(target.name)
                .arg(damage);
            
            if (isCritical) {
                logMessage += " (暴击!)";
            }
            
            addBattleLog(logMessage);
            
            // 检查敌人是否死亡
            if (target.health <= 0) {
                addBattleLog(QString("%1 被击败了！").arg(target.name));
                emit enemyDefeated(target.id);
            }
            
            // 发送敌人状态更新信号
            emit enemiesChanged();
            
            // 结束玩家回合
            endPlayerTurn();
        });
    } else {
        // 简化的本地伤害计算
        int damage = qMax(1, m_player->getAttack() - target.defense / 2);
        
        // 随机暴击
        bool isCritical = QRandomGenerator::global()->bounded(100) < 10; // 10%暴击率
        if (isCritical) {
            damage *= 2;
        }
        
        target.health = qMax(0, target.health - damage);
        
        QString logMessage = QString("%1 攻击 %2 造成了 %3 点伤害")
            .arg(m_player->getName())
            .arg(target.name)
            .arg(damage);
        
        if (isCritical) {
            logMessage += " (暴击!)";
        }
        
        addBattleLog(logMessage);
        
        if (target.health <= 0) {
            addBattleLog(QString("%1 被击败了！").arg(target.name));
            emit enemyDefeated(target.id);
        }
        
        emit enemiesChanged();
        endPlayerTurn();
    }
}

void BattleSystem::executePlayerItemUse(int itemId, int targetId)
{
    Q_UNUSED(itemId);
    // 简化的物品使用逻辑
    QString itemName = "治疗药水"; // 暂时硬编码
    
    if (targetId == -1) { // 对自己使用
        m_player->heal(50);
        addBattleLog(QString("%1 使用了 %2，恢复了50点生命值")
            .arg(m_player->getName())
            .arg(itemName));
    }
    
    endPlayerTurn();
}

void BattleSystem::executeEnemyActions()
{
    // 为每个存活的敌人执行行动
    for (Enemy &enemy : m_enemies) {
        if (enemy.health <= 0) {
            continue;
        }
        
        executeEnemyAction(enemy);
    }
    
    // 延迟结束敌人回合，让玩家看到结果
    QTimer::singleShot(2000 / m_battleSpeed, this, &BattleSystem::endEnemyTurn);
}

void BattleSystem::executeEnemyAction(Enemy &enemy)
{
    // 简化的敌人AI：总是攻击玩家
    int damage = qMax(1, enemy.attack - m_player->getDefense() / 2);
    
    // 随机波动
    damage += QRandomGenerator::global()->bounded(-2, 3);
    damage = qMax(1, damage);
    
    bool playerDied = m_player->takeDamage(damage);
    
    addBattleLog(QString("%1 攻击了 %2，造成了 %3 点伤害")
        .arg(enemy.name)
        .arg(m_player->getName())
        .arg(damage));
    
    if (playerDied) {
        addBattleLog(QString("%1 倒下了！").arg(m_player->getName()));
    }
}

void BattleSystem::executeAutoAction()
{
    if (m_battleState != BattleState::PlayerTurn || !m_autoMode) {
        return;
    }
    
    // 简化的自动战斗逻辑：攻击第一个存活的敌人
    for (const Enemy &enemy : m_enemies) {
        if (enemy.health > 0) {
            playerAttack(enemy.id, 0); // 使用基础攻击
            break;
        }
    }
}

bool BattleSystem::checkBattleEnd()
{
    // 检查玩家是否死亡
    if (m_player->getHealth() <= 0) {
        m_battleState = BattleState::Defeat;
        addBattleLog("战斗失败！");
        emit battleDefeated();
        endBattle();
        return true;
    }
    
    // 检查是否所有敌人都被击败
    bool allEnemiesDefeated = true;
    for (const Enemy &enemy : m_enemies) {
        if (enemy.health > 0) {
            allEnemiesDefeated = false;
            break;
        }
    }
    
    if (allEnemiesDefeated) {
        m_battleState = BattleState::Victory;
        addBattleLog("战斗胜利！");
        
        // 计算经验奖励
        int expReward = 0;
        for (const Enemy &enemy : m_enemies) {
            expReward += enemy.level * 10;
        }
        
        m_player->addExperience(expReward);
        addBattleLog(QString("获得了 %1 点经验值").arg(expReward));
        
        emit battleWon();
        endBattle();
        return true;
    }
    
    return false;
}

void BattleSystem::addBattleLog(const QString &message)
{
    m_battleLog.append(message);
    emit battleLogChanged();
    
    qDebug() << "BattleSystem:" << message;
}

void BattleSystem::onTurnTimeout()
{
    if (m_battleState == BattleState::PlayerTurn) {
        addBattleLog(QString("%1 的回合时间用完了").arg(m_player->getName()));
        endPlayerTurn();
    }
}

void BattleSystem::onPlayerDied()
{
    if (m_battleState != BattleState::Idle) {
        checkBattleEnd();
    }
}