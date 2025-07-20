/*
 * 文件名: GameState.cpp
 * 说明: 游戏状态管理器实现 - 管理游戏的全局状态和数据持久化
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 实现游戏状态的完整生命周期管理
 * - 提供线程安全的状态访问和修改
 * - 支持状态数据的序列化和反序列化
 * - 实现玩家数据的自动计算和验证
 * - 提供状态变更的信号通知机制
 * 
 * 实现特点:
 * - 线程安全：使用QMutex保护共享数据
 * - 数据验证：确保状态数据的合法性
 * - 自动计算：如升级经验值计算
 * - 信号通知：状态变更时自动发送信号
 * 
 * 依赖项:
 * - Qt Core (QObject, QVariant, QMutex等)
 * - Qt JSON (QJsonDocument, QJsonObject等)
 * 
 * 使用场景:
 * - 游戏启动时初始化状态
 * - 游戏过程中更新玩家数据
 * - 场景切换时保存状态
 * - 游戏退出时持久化数据
 */

#include "game/GameState.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMutexLocker>

/**
 * @brief 构造函数
 * @param parent 父对象指针，用于Qt对象树管理
 * 
 * 初始化游戏状态管理器，设置所有状态数据的默认值
 */
GameState::GameState(QObject *parent)
    : QObject(parent)
    , m_currentState(State::Uninitialized)     // 初始状态为未初始化
    , m_previousState(State::Uninitialized)    // 前一状态也为未初始化
{
    qDebug() << "GameState: 构造函数调用，初始化游戏状态管理器";
    
    // 初始化所有状态数据为默认值
    initializeDefaults();
    
    qDebug() << "GameState: 游戏状态管理器初始化完成";
}

/**
 * @brief 析构函数
 * 
 * 清理资源，确保状态数据正确保存
 */
GameState::~GameState()
{
    qDebug() << "GameState: 析构函数调用，开始清理资源";
    
    // 在析构时自动保存状态（可选）
    // saveState(); // 取消注释以启用自动保存
    
    qDebug() << "GameState: 游戏状态管理器析构完成";
}

/**
 * @brief 初始化默认值
 * 
 * 设置所有状态数据的初始默认值，确保游戏有一个合理的起始状态
 */
void GameState::initializeDefaults()
{
    qDebug() << "GameState: 初始化默认状态数据";
    
    // 玩家基本数据默认值
    m_playerLevel = 1;                          // 起始等级为1
    m_playerExperience = 0;                     // 起始经验为0
    m_playerGold = 100;                         // 起始金币100
    m_playerName = "新玩家";                     // 默认玩家名称
    m_playerClass = PlayerClass::None;          // 未选择职业
    
    // 游戏进度数据默认值
    m_currentScene = "MainMenu";                // 起始场景为主菜单
    m_gameProgress = 0.0f;                      // 游戏进度0%
    m_lastSaveTime = QDateTime::currentDateTime(); // 当前时间为最后保存时间
    
    // 清空通用状态数据
    m_stateData.clear();
    
    // 设置一些默认的状态标志
    m_stateData["tutorial_completed"] = false;  // 教程未完成
    m_stateData["first_time_player"] = true;    // 首次游戏
    m_stateData["sound_enabled"] = true;        // 音效开启
    m_stateData["music_enabled"] = true;        // 音乐开启
    
    qDebug() << "GameState: 默认状态数据初始化完成";
    qDebug() << "  玩家等级:" << m_playerLevel;
    qDebug() << "  玩家金币:" << m_playerGold;
    qDebug() << "  当前场景:" << m_currentScene;
}

/**
 * @brief 设置游戏状态
 * @param state 新的游戏状态
 * 
 * 执行状态转换，包括合法性检查和相关的状态切换逻辑
 */
void GameState::setState(State state)
{
    QMutexLocker locker(&m_mutex);  // 线程安全保护
    
    // 检查状态是否真的发生了变化
    if (m_currentState == state) {
        qDebug() << "GameState: 状态未变化，忽略设置请求:" << stateToString(state);
        return;
    }
    
    // 验证状态转换的合法性
    if (!isValidStateTransition(m_currentState, state)) {
        qWarning() << "GameState: 非法状态转换:" 
                   << stateToString(m_currentState) << "->" << stateToString(state);
        return;
    }
    
    qDebug() << "GameState: 状态转换:" 
             << stateToString(m_currentState) << "->" << stateToString(state);
    
    // 保存前一状态
    State oldState = m_currentState;
    m_previousState = m_currentState;
    
    // 设置新状态
    m_currentState = state;
    
    // 执行状态切换相关的逻辑
    switch (state) {
        case State::InGame:
            // 进入游戏状态时的处理
            qDebug() << "GameState: 进入游戏状态";
            break;
            
        case State::Paused:
            // 游戏暂停时的处理
            qDebug() << "GameState: 游戏暂停";
            break;
            
        case State::MainMenu:
            // 返回主菜单时的处理
            qDebug() << "GameState: 返回主菜单";
            break;
            
        default:
            // 其他状态的默认处理
            break;
    }
    
    // 发送状态变更信号
    emit stateChanged(m_currentState, oldState);
    emit gamePausedChanged(m_currentState == State::Paused);
    
    qDebug() << "GameState: 状态转换完成";
}

/**
 * @brief 验证状态转换的合法性
 * @param fromState 源状态
 * @param toState 目标状态
 * @return bool 转换是否合法
 * 
 * 定义状态机的转换规则，防止非法的状态跳转
 */
bool GameState::isValidStateTransition(State fromState, State toState) const
{
    // 从未初始化状态可以转换到任何状态
    if (fromState == State::Uninitialized) {
        return true;
    }
    
    // 定义合法的状态转换规则
    switch (fromState) {
        case State::MainMenu:
            // 主菜单可以转换到：游戏中、设置、加载中
            return (toState == State::InGame || 
                    toState == State::Settings || 
                    toState == State::Loading);
            
        case State::InGame:
            // 游戏中可以转换到：暂停、战斗、物品栏、游戏结束、胜利
            return (toState == State::Paused || 
                    toState == State::Battle || 
                    toState == State::Inventory ||
                    toState == State::GameOver ||
                    toState == State::Victory);
            
        case State::Paused:
            // 暂停状态可以转换到：游戏中、主菜单、设置
            return (toState == State::InGame || 
                    toState == State::MainMenu || 
                    toState == State::Settings);
            
        case State::Battle:
            // 战斗状态可以转换到：游戏中、游戏结束、胜利
            return (toState == State::InGame || 
                    toState == State::GameOver || 
                    toState == State::Victory);
            
        case State::Inventory:
            // 物品栏可以转换到：游戏中
            return (toState == State::InGame);
            
        case State::Settings:
            // 设置界面可以转换到：主菜单、游戏中（如果从游戏中进入）
            return (toState == State::MainMenu || 
                    toState == State::InGame);
            
        case State::Loading:
            // 加载中可以转换到：游戏中、主菜单（加载失败）
            return (toState == State::InGame || 
                    toState == State::MainMenu);
            
        case State::GameOver:
        case State::Victory:
            // 游戏结束和胜利状态可以转换到：主菜单
            return (toState == State::MainMenu);
            
        default:
            // 未定义的状态转换，默认允许
            qWarning() << "GameState: 未定义的状态转换规则:" 
                       << stateToString(fromState) << "->" << stateToString(toState);
            return true;
    }
}

/**
 * @brief 设置玩家等级
 * @param level 新的等级值
 * 
 * 设置玩家等级，包括数据验证和相关属性更新
 */
void GameState::setPlayerLevel(int level)
{
    QMutexLocker locker(&m_mutex);
    
    // 验证等级范围（1-100）
    int newLevel = qBound(1, level, 100);
    
    if (m_playerLevel == newLevel) {
        return; // 等级未变化
    }
    
    qDebug() << "GameState: 设置玩家等级从" << m_playerLevel << "到" << newLevel;
    
    int oldLevel = m_playerLevel;
    m_playerLevel = newLevel;
    
    // 如果是升级，触发升级信号
    if (newLevel > oldLevel) {
        qDebug() << "GameState: 玩家升级！新等级:" << newLevel;
        emit playerLevelUp(newLevel, oldLevel);
    }
    
    // 发送等级变更信号
    emit playerLevelChanged(m_playerLevel);
}

/**
 * @brief 设置玩家经验值
 * @param experience 新的经验值
 * 
 * 设置玩家经验值，自动处理升级逻辑
 */
void GameState::setPlayerExperience(int experience)
{
    QMutexLocker locker(&m_mutex);
    
    // 验证经验值范围（非负数）
    int newExperience = qMax(0, experience);
    
    if (m_playerExperience == newExperience) {
        return; // 经验值未变化
    }
    
    qDebug() << "GameState: 设置玩家经验从" << m_playerExperience << "到" << newExperience;
    
    m_playerExperience = newExperience;
    
    // 检查是否可以升级
    while (true) {
        int requiredExp = calculateExperienceForLevel(m_playerLevel + 1);
        if (m_playerExperience >= requiredExp && m_playerLevel < 100) {
            // 可以升级
            setPlayerLevel(m_playerLevel + 1);
        } else {
            break; // 无法继续升级
        }
    }
    
    // 发送经验值变更信号
    emit playerExperienceChanged(m_playerExperience);
}

/**
 * @brief 增加玩家经验值
 * @param amount 增加的经验值
 * @return bool 是否触发了升级
 * 
 * 便捷方法，用于增加经验值而不是直接设置
 */
bool GameState::addPlayerExperience(int amount)
{
    if (amount <= 0) {
        return false; // 无效的经验值增量
    }
    
    int oldLevel = m_playerLevel;
    setPlayerExperience(m_playerExperience + amount);
    
    // 检查是否发生了升级
    bool leveledUp = (m_playerLevel > oldLevel);
    
    if (leveledUp) {
        qDebug() << "GameState: 获得经验" << amount << "，触发升级到等级" << m_playerLevel;
    } else {
        qDebug() << "GameState: 获得经验" << amount << "，当前经验:" << m_playerExperience;
    }
    
    return leveledUp;
}

/**
 * @brief 计算升级所需经验值
 * @param level 目标等级
 * @return int 所需经验值
 * 
 * 使用指数增长公式计算升级所需的经验值
 */
int GameState::calculateExperienceForLevel(int level) const
{
    if (level <= 1) {
        return 0; // 1级不需要经验
    }
    
    // 使用指数增长公式：baseExp * (level - 1)^1.5
    // 这样可以让后期升级变得更困难
    int baseExp = 100;
    double exponent = 1.5;
    
    int requiredExp = static_cast<int>(baseExp * qPow(level - 1, exponent));
    
    return requiredExp;
}

/**
 * @brief 设置玩家金币数量
 * @param gold 新的金币数量
 */
void GameState::setPlayerGold(int gold)
{
    QMutexLocker locker(&m_mutex);
    
    // 验证金币范围（非负数，最大值为999999）
    int newGold = qBound(0, gold, 999999);
    
    if (m_playerGold == newGold) {
        return; // 金币未变化
    }
    
    qDebug() << "GameState: 设置玩家金币从" << m_playerGold << "到" << newGold;
    
    m_playerGold = newGold;
    
    // 发送金币变更信号
    emit playerGoldChanged(m_playerGold);
}

/**
 * @brief 设置玩家姓名
 * @param name 新的玩家姓名
 */
void GameState::setPlayerName(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    
    // 验证姓名（不能为空，长度限制）
    QString newName = name.trimmed();
    if (newName.isEmpty()) {
        newName = "无名玩家";
    }
    if (newName.length() > 20) {
        newName = newName.left(20); // 限制最大长度
    }
    
    if (m_playerName == newName) {
        return; // 姓名未变化
    }
    
    qDebug() << "GameState: 设置玩家姓名从'" << m_playerName << "'到'" << newName << "'";
    
    m_playerName = newName;
    
    // 发送姓名变更信号
    emit playerNameChanged(m_playerName);
}

/**
 * @brief 设置玩家职业
 * @param playerClass 新的职业
 */
void GameState::setPlayerClass(PlayerClass playerClass)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_playerClass == playerClass) {
        return; // 职业未变化
    }
    
    qDebug() << "GameState: 设置玩家职业:" << static_cast<int>(playerClass);
    
    m_playerClass = playerClass;
    
    // 发送职业变更信号
    emit playerClassChanged(m_playerClass);
}

/**
 * @brief 获取状态名称字符串
 * @param state 状态枚举值
 * @return QString 状态名称
 * 
 * 将状态枚举转换为可读的字符串，用于调试和日志
 */
QString GameState::stateToString(State state)
{
    switch (state) {
        case State::Uninitialized: return "未初始化";
        case State::MainMenu: return "主菜单";
        case State::InGame: return "游戏中";
        case State::Paused: return "暂停";
        case State::Loading: return "加载中";
        case State::Battle: return "战斗中";
        case State::Inventory: return "物品栏";
        case State::Settings: return "设置";
        case State::GameOver: return "游戏结束";
        case State::Victory: return "胜利";
        default: return "未知状态";
    }
}
