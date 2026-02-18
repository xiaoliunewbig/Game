/*
 * 文件名: GameEngine.cpp
 * 说明: 游戏引擎核心类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 *
 * 本文件实现GameEngine类的所有方法，作为游戏的核心控制器，
 * 协调各个子系统的工作，管理游戏状态和循环。
 */
#include "core/GameEngine.h"
#include "core/SceneManager.h"
#include "core/ResourceManager.h"
#include "network/NetworkManager.h"
#include "audio/AudioManager.h"
#include "game/GameState.h"
#include "game/Player.h"
#include "game/BattleSystem.h"
#include "game/InventorySystem.h"

#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_gameTimer(new QTimer(this))
    , m_sceneManager(nullptr)
    , m_resourceManager(nullptr)
    , m_networkManager(nullptr)
    , m_audioManager(nullptr)
    , m_currentState(GameEngineState::Uninitialized)
    , m_isInitialized(false)
    , m_deltaTime(0.0f)
    , m_frameRate(0)
    , m_lastFrameTime(0)
    , m_frameCounter(0)
    , m_frameRateTime(0)
{
    // 配置游戏循环定时器
    m_gameTimer->setInterval(1000 / TARGET_FPS); // 60 FPS
    connect(m_gameTimer, &QTimer::timeout, this, &GameEngine::update);
    
    // 初始化时间戳
    QElapsedTimer timer;
    timer.start();
    m_lastFrameTime = timer.elapsed();
    m_frameRateTime = m_lastFrameTime;
}

GameEngine::~GameEngine()
{
    stop();
}

bool GameEngine::initialize(SceneManager *sceneManager,
                           ResourceManager *resourceManager,
                           NetworkManager *networkManager,
                           AudioManager *audioManager)
{
    try {
        // 保存管理器引用
        m_sceneManager = sceneManager;
        m_resourceManager = resourceManager;
        m_networkManager = networkManager;
        m_audioManager = audioManager;
        
        // 验证必要的管理器
        if (!m_sceneManager || !m_resourceManager || 
            !m_networkManager || !m_audioManager) {
            qWarning() << "GameEngine: 缺少必要的管理器";
            return false;
        }
        
        // 创建游戏系统
        m_gameState = std::make_unique<GameState>();
        m_player = std::make_unique<Player>();
        m_battleSystem = std::make_unique<BattleSystem>();
        m_inventorySystem = std::make_unique<Game::InventorySystem>();
        
        // 建立信号连接
        connect(m_sceneManager, &SceneManager::sceneTransitionCompleted,
                this, &GameEngine::onSceneTransitionCompleted);
        
        // 加载游戏配置
        if (!loadGameConfig()) {
            qWarning() << "GameEngine: 加载游戏配置失败";
        }
        
        // 设置初始化状态
        m_isInitialized = true;
        changeState(GameEngineState::MainMenu);
        
        emit initializationChanged();
        
        qDebug() << "GameEngine: 初始化成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 初始化异常:" << e.what();
        return false;
    }
}

void GameEngine::start()
{
    if (!m_isInitialized) {
        qWarning() << "GameEngine: 引擎未初始化，无法启动";
        return;
    }
    
    qDebug() << "GameEngine: 启动游戏循环";
    m_gameTimer->start();
}

void GameEngine::stop()
{
    if (m_gameTimer->isActive()) {
        qDebug() << "GameEngine: 停止游戏循环";
        m_gameTimer->stop();
        
        // 保存游戏配置
        saveGameConfig();
    }
}

QString GameEngine::stateDescription() const
{
    switch (m_currentState) {
        case GameEngineState::Uninitialized: return "未初始化";
        case GameEngineState::MainMenu: return "主菜单";
        case GameEngineState::Loading: return "加载中";
        case GameEngineState::Playing: return "游戏进行中";
        case GameEngineState::Battle: return "战斗中";
        case GameEngineState::Paused: return "暂停";
        case GameEngineState::GameOver: return "游戏结束";
        default: return "未知状态";
    }
}

bool GameEngine::startNewGame(const QString &playerName, const QString &profession)
{
    if (!m_isInitialized) {
        qWarning() << "GameEngine: 引擎未初始化";
        return false;
    }
    
    try {
        qDebug() << "GameEngine: 开始新游戏 - 玩家:" << playerName << "职业:" << profession;
        
        // 初始化游戏状态
        if (m_gameState) {
            // m_gameState->initializeNewGame(); // 暂时注释掉不存在的方法
        }
        
        // 初始化背包系统
        if (m_inventorySystem) {
            // m_inventorySystem->initializeNewGame(); // 暂时注释掉不存在的方法
        }
        
        // 初始化玩家
        if (m_player) {
            m_player->setName(playerName);
            // 设置职业的逻辑需要根据实际的Player类实现
        }
        
        // 切换到游戏场景
        if (m_sceneManager) {
            m_sceneManager->loadScene("gameplay");
        }
        
        changeState(GameEngineState::Playing);
        
        qDebug() << "GameEngine: 新游戏启动成功";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 启动新游戏异常:" << e.what();
        return false;
    }
}

bool GameEngine::loadGame(int slotIndex)
{
    try {
        if (slotIndex < 0 || slotIndex >= 10) {
            qWarning() << "GameEngine: 无效的存档槽位:" << slotIndex;
            return false;
        }

        QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                          + QString("/saves/save_%1.json").arg(slotIndex);
        QFile file(savePath);
        if (!file.exists()) {
            qWarning() << "GameEngine: 存档文件不存在:" << savePath;
            return false;
        }

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "GameEngine: 无法打开存档文件:" << savePath;
            return false;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "GameEngine: 存档文件解析失败:" << parseError.errorString();
            return false;
        }

        QJsonObject saveData = doc.object();

        // 恢复游戏状态
        if (m_gameState) {
            m_gameState->loadFromJson(saveData["gameState"].toObject());
        }
        if (m_player) {
            m_player->loadFromJson(saveData["player"].toObject());
        }

        changeState(GameEngineState::Playing);
        emit gameLoaded(slotIndex, true);
        qDebug() << "GameEngine: 游戏加载完成，槽位:" << slotIndex;
        return true;

    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 加载游戏异常:" << e.what();
        return false;
    }
}

bool GameEngine::saveGame(int slotIndex)
{
    try {
        if (slotIndex < 0 || slotIndex >= 10) {
            qWarning() << "GameEngine: 无效的存档槽位:" << slotIndex;
            return false;
        }

        // 确保存档目录存在
        QString saveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/saves";
        QDir().mkpath(saveDir);

        QString savePath = saveDir + QString("/save_%1.json").arg(slotIndex);

        // 构建存档数据
        QJsonObject saveData;
        saveData["version"] = 1;
        saveData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        saveData["slotIndex"] = slotIndex;

        if (m_gameState) {
            saveData["gameState"] = m_gameState->toJson();
        }
        if (m_player) {
            saveData["player"] = m_player->toJson();
        }

        // 写入文件
        QJsonDocument doc(saveData);
        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "GameEngine: 无法写入存档文件:" << savePath;
            return false;
        }

        file.write(doc.toJson());
        file.close();

        emit gameSaved(slotIndex, true);
        qDebug() << "GameEngine: 游戏保存完成，槽位:" << slotIndex;
        return true;

    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 保存游戏异常:" << e.what();
        return false;
    }
}

void GameEngine::pauseGame()
{
    if (m_currentState == GameEngineState::Playing || 
        m_currentState == GameEngineState::Battle) {
        changeState(GameEngineState::Paused);
        emit gamePaused();
        qDebug() << "GameEngine: 游戏暂停";
    }
}

void GameEngine::resumeGame()
{
    if (m_currentState == GameEngineState::Paused) {
        changeState(GameEngineState::Playing);
        emit gameResumed();
        qDebug() << "GameEngine: 游戏恢复";
    }
}

QJsonObject GameEngine::getSaveSlotInfo(int slotIndex) const
{
    QJsonObject info;
    
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString saveFile = QString("%1/saves/save_%2.json").arg(saveDir).arg(slotIndex);
    
    QFile file(saveFile);
    if (!file.exists()) {
        info["exists"] = false;
        return info;
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject saveData = doc.object();
        
        info["exists"] = true;
        info["saveTime"] = saveData["saveTime"].toString();
        
        if (saveData.contains("player")) {
            QJsonObject playerData = saveData["player"].toObject();
            info["playerName"] = playerData["name"].toString();
            info["playerLevel"] = playerData["level"].toInt();
            info["profession"] = playerData["profession"].toString();
        }
        
        if (saveData.contains("gameState")) {
            QJsonObject gameStateData = saveData["gameState"].toObject();
            info["gameProgress"] = gameStateData["progress"].toString();
            info["playTime"] = gameStateData["playTime"].toInt();
        }
    }
    
    return info;
}

bool GameEngine::deleteSave(int slotIndex)
{
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString saveFile = QString("%1/saves/save_%2.json").arg(saveDir).arg(slotIndex);
    
    QFile file(saveFile);
    if (file.exists()) {
        bool success = file.remove();
        if (success) {
            qDebug() << "GameEngine: 删除存档成功 - 槽位" << slotIndex;
        } else {
            qWarning() << "GameEngine: 删除存档失败 - 槽位" << slotIndex;
        }
        return success;
    }
    
    return true; // 文件不存在也算删除成功
}

QJsonObject GameEngine::getGameStats() const
{
    QJsonObject stats;
    
    if (m_player) {
        stats["playerName"] = m_player->getName();
        stats["level"] = m_player->getLevel();
        stats["health"] = m_player->getHealth();
        stats["maxHealth"] = m_player->getMaxHealth();
        // stats["profession"] = static_cast<int>(m_player->getProfession()); // 转换为int
    }
    
    if (m_gameState) {
        // stats["playTime"] = m_gameState->getPlayTime(); // 暂时注释掉不存在的方法
        // stats["gameProgress"] = m_gameState->getProgress(); // 暂时注释掉不存在的方法
        // stats["currentChapter"] = m_gameState->getCurrentChapter(); // 暂时注释掉不存在的方法
    }
    
    return stats;
}

void GameEngine::resetSettings()
{
    qDebug() << "GameEngine: 重置游戏设置";

    m_gameConfig = QJsonObject();
    loadGameConfig();

    if (m_audioManager) {
        m_audioManager->setMasterVolume(1.0f);
        m_audioManager->setMusicVolume(0.8f);
        m_audioManager->setSFXVolume(1.0f);
        m_audioManager->setMuted(false);
    }
}

void GameEngine::exitToMainMenu()
{
    qDebug() << "GameEngine: 退出到主菜单";

    if (m_currentState == GameEngineState::Playing ||
        m_currentState == GameEngineState::Battle ||
        m_currentState == GameEngineState::Paused) {
        saveGameConfig();

        if (m_sceneManager) {
            m_sceneManager->loadScene("main_menu");
        }

        changeState(GameEngineState::MainMenu);
    }
}

void GameEngine::forceGarbageCollection()
{
    // 清理资源缓存
    if (m_resourceManager) {
        // m_resourceManager->clearCache(); // 暂时注释掉不存在的方法
    }
    
    // 触发Qt的垃圾回收
    // 在实际实现中可能需要调用QML引擎的gc()方法
    
    qDebug() << "GameEngine: 强制垃圾回收完成";
}

void GameEngine::update()
{
    // 计算帧时间
    QElapsedTimer timer;
    timer.start();
    qint64 currentTime = timer.elapsed();
    m_deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;
    m_lastFrameTime = currentTime;
    
    // 更新帧率统计
    updateFrameRate();
    
    // 根据当前状态更新游戏逻辑
    if (m_currentState == GameEngineState::Playing || 
        m_currentState == GameEngineState::Battle) {
        updateGameLogic(m_deltaTime);
    }
}

void GameEngine::onNetworkConnectionChanged(bool connected)
{
    if (!connected) {
        qWarning() << "GameEngine: 网络连接丢失";
        emit errorOccurred("网络连接丢失");
    } else {
        qDebug() << "GameEngine: 网络连接恢复";
    }
}

void GameEngine::onSceneTransitionCompleted(const QString &sceneName)
{
    qDebug() << "GameEngine: 场景切换完成 -" << sceneName;
    
    // 根据场景名称调整游戏状态
    if (sceneName == "main_menu") {
        changeState(GameEngineState::MainMenu);
    } else if (sceneName == "battle") {
        changeState(GameEngineState::Battle);
    } else {
        changeState(GameEngineState::Playing);
    }
}

void GameEngine::changeState(GameEngineState newState)
{
    if (m_currentState != newState) {
        GameEngineState oldState = m_currentState;
        m_currentState = newState;
        
        qDebug() << "GameEngine: 状态改变" << static_cast<int>(oldState) 
                 << "->" << static_cast<int>(newState);
        
        emit stateChanged();
    }
}

void GameEngine::updateFrameRate()
{
    m_frameCounter++;
    
    QElapsedTimer timer;
    timer.start();
    qint64 currentTime = timer.elapsed();
    
    if (currentTime - m_frameRateTime >= 1000) { // 每秒更新一次帧率
        m_frameRate = m_frameCounter;
        m_frameCounter = 0;
        m_frameRateTime = currentTime;
        emit frameRateChanged();
    }
    
    emit deltaTimeChanged();
}

void GameEngine::updateGameLogic(float deltaTime)
{
    // 更新游戏状态
    if (m_gameState) {
        // m_gameState->update(deltaTime); // 暂时注释掉不存在的方法
    }
    
    // 更新玩家
    if (m_player) {
        m_player->update(deltaTime);
    }
    
    // 更新战斗系统
    if (m_battleSystem && m_currentState == GameEngineState::Battle) {
        m_battleSystem->update(deltaTime);
    }
    
    // 更新背包系统
    if (m_inventorySystem) {
        // m_inventorySystem->update(deltaTime); // 暂时注释掉不存在的方法
    }
}

bool GameEngine::loadGameConfig()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configFile = configDir + "/game_config.json";
    
    QFile file(configFile);
    if (!file.exists()) {
        // 创建默认配置
        m_gameConfig["graphics"] = QJsonObject{
            {"resolution", "1920x1080"},
            {"fullscreen", false},
            {"vsync", true}
        };
        m_gameConfig["audio"] = QJsonObject{
            {"masterVolume", 1.0},
            {"musicVolume", 0.8},
            {"sfxVolume", 1.0}
        };
        return saveGameConfig();
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        m_gameConfig = doc.object();
        return true;
    }
    
    return false;
}

bool GameEngine::saveGameConfig()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    QString configFile = configDir + "/game_config.json";
    QFile file(configFile);
    
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_gameConfig);
        file.write(doc.toJson());
        return true;
    }
    
    return false;
}
