/*
 * 文件名: GameEngine.cpp
 * 说明: 游戏引擎核心类的具体实现。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
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
        m_inventorySystem = std::make_unique<InventorySystem>();
        
        // 建立信号连接
        connect(m_networkManager, &NetworkManager::connectionChanged,
                this, &GameEngine::onNetworkConnectionChanged);
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
    try {
        changeState(GameEngineState::Loading);
        
        // 初始化玩家数据
        if (m_player) {
            m_player->setName(playerName);
            m_player->setProfession(profession);
            m_player->initializeNewPlayer();
        }
        
        // 初始化游戏状态
        if (m_gameState) {
            m_gameState->initializeNewGame();
        }
        
        // 初始化背包系统
        if (m_inventorySystem) {
            m_inventorySystem->initializeNewGame();
        }
        
        // 切换到游戏场景
        if (m_sceneManager) {
            m_sceneManager->loadScene("game_world");
        }
        
        changeState(GameEngineState::Playing);
        emit newGameStarted(playerName, profession);
        
        qDebug() << "GameEngine: 新游戏开始 -" << playerName << profession;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 开始新游戏失败:" << e.what();
        emit errorOccurred("开始新游戏失败");
        changeState(GameEngineState::MainMenu);
        return false;
    }
}

bool GameEngine::loadGame(int saveSlot)
{
    try {
        changeState(GameEngineState::Loading);
        
        // 构建存档文件路径
        QString saveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString saveFile = QString("%1/saves/save_%2.json").arg(saveDir).arg(saveSlot);
        
        QFile file(saveFile);
        if (!file.exists()) {
            qWarning() << "GameEngine: 存档文件不存在:" << saveFile;
            emit gameLoaded(saveSlot, false);
            changeState(GameEngineState::MainMenu);
            return false;
        }
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "GameEngine: 无法打开存档文件:" << saveFile;
            emit gameLoaded(saveSlot, false);
            changeState(GameEngineState::MainMenu);
            return false;
        }
        
        // 解析存档数据
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject saveData = doc.object();
        
        // 加载玩家数据
        if (m_player && saveData.contains("player")) {
            m_player->loadFromJson(saveData["player"].toObject());
        }
        
        // 加载游戏状态
        if (m_gameState && saveData.contains("gameState")) {
            m_gameState->loadFromJson(saveData["gameState"].toObject());
        }
        
        // 加载背包数据
        if (m_inventorySystem && saveData.contains("inventory")) {
            m_inventorySystem->loadFromJson(saveData["inventory"].toObject());
        }
        
        // 切换到游戏场景
        QString sceneName = saveData["currentScene"].toString("game_world");
        if (m_sceneManager) {
            m_sceneManager->loadScene(sceneName);
        }
        
        changeState(GameEngineState::Playing);
        emit gameLoaded(saveSlot, true);
        
        qDebug() << "GameEngine: 游戏加载成功 - 槽位" << saveSlot;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 加载游戏失败:" << e.what();
        emit errorOccurred("加载游戏失败");
        emit gameLoaded(saveSlot, false);
        changeState(GameEngineState::MainMenu);
        return false;
    }
}

bool GameEngine::saveGame(int saveSlot)
{
    try {
        // 构建存档数据
        QJsonObject saveData;
        
        if (m_player) {
            saveData["player"] = m_player->toJson();
        }
        
        if (m_gameState) {
            saveData["gameState"] = m_gameState->toJson();
        }
        
        if (m_inventorySystem) {
            saveData["inventory"] = m_inventorySystem->toJson();
        }
        
        if (m_sceneManager) {
            saveData["currentScene"] = m_sceneManager->getCurrentSceneName();
        }
        
        saveData["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        saveData["version"] = "1.0.0";
        
        // 确保存档目录存在
        QString saveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(saveDir + "/saves");
        
        // 写入存档文件
        QString saveFile = QString("%1/saves/save_%2.json").arg(saveDir).arg(saveSlot);
        QFile file(saveFile);
        
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "GameEngine: 无法创建存档文件:" << saveFile;
            emit gameSaved(saveSlot, false);
            return false;
        }
        
        QJsonDocument doc(saveData);
        file.write(doc.toJson());
        
        emit gameSaved(saveSlot, true);
        qDebug() << "GameEngine: 游戏保存成功 - 槽位" << saveSlot;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "GameEngine: 保存游戏失败:" << e.what();
        emit errorOccurred("保存游戏失败");
        emit gameSaved(saveSlot, false);
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
        stats["playerLevel"] = m_player->getLevel();
        stats["playerExp"] = m_player->getExperience();
        stats["playerName"] = m_player->getName();
        stats["profession"] = m_player->getProfession();
    }
    
    if (m_gameState) {
        stats["playTime"] = m_gameState->getPlayTime();
        stats["gameProgress"] = m_gameState->getProgress();
        stats["currentChapter"] = m_gameState->getCurrentChapter();
    }
    
    stats["frameRate"] = m_frameRate;
    stats["deltaTime"] = m_deltaTime;
    stats["currentState"] = stateDescription();
    
    return stats;
}

void GameEngine::resetSettings()
{
    m_gameConfig = QJsonObject();
    saveGameConfig();
    qDebug() << "GameEngine: 游戏设置已重置";
}

void GameEngine::exitToMainMenu()
{
    // 自动保存当前进度
    if (m_currentState == GameEngineState::Playing || 
        m_currentState == GameEngineState::Battle) {
        saveGame(0); // 自动保存到槽位0
    }
    
    // 切换到主菜单场景
    if (m_sceneManager) {
        m_sceneManager->loadScene("main_menu");
    }
    
    changeState(GameEngineState::MainMenu);
    qDebug() << "GameEngine: 退出到主菜单";
}

void GameEngine::forceGarbageCollection()
{
    // 清理资源缓存
    if (m_resourceManager) {
        m_resourceManager->clearCache();
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
        m_gameState->update(deltaTime);
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
        m_inventorySystem->update(deltaTime);
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