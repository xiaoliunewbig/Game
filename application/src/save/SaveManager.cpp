/*
 * 文件名: SaveManager.cpp
 * 说明: 存档管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件实现游戏存档的保存、加载和管理功能。
 */

#include "save/SaveManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

SaveManager::SaveManager(QObject *parent)
    : QObject(parent)
{
    initializeSaveDirectory();
}

bool SaveManager::saveGame(int slot, const Player *player, const GameState *gameState)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        qWarning() << "SaveManager: 无效的存档槽位:" << slot;
        return false;
    }
    
    if (!player || !gameState) {
        qWarning() << "SaveManager: 玩家或游戏状态为空";
        return false;
    }
    
    QJsonObject saveData;
    
    // 保存元数据
    QJsonObject metadata;
    metadata["version"] = "1.0.0";
    metadata["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["slot"] = slot;
    saveData["metadata"] = metadata;
    
    // 保存玩家数据
    QJsonObject playerData;
    playerData["name"] = player->getName();
    playerData["level"] = player->getLevel();
    playerData["experience"] = player->getExperience();
    playerData["profession"] = static_cast<int>(player->getProfession());
    playerData["health"] = player->getHealth();
    playerData["maxHealth"] = player->getMaxHealth();
    playerData["mana"] = player->getMana();
    playerData["maxMana"] = player->getMaxMana();
    playerData["attack"] = player->getAttack();
    playerData["defense"] = player->getDefense();
    playerData["speed"] = player->getSpeed();
    playerData["luck"] = player->getLuck();
    
    // 保存状态效果
    QJsonArray statusEffects;
    for (const auto &effect : player->getStatusEffects()) {
        QJsonObject effectObj;
        effectObj["type"] = static_cast<int>(effect.type);
        effectObj["duration"] = effect.duration;
        effectObj["value"] = effect.value;
        statusEffects.append(effectObj);
    }
    playerData["statusEffects"] = statusEffects;
    
    saveData["player"] = playerData;
    
    // 保存游戏状态
    QJsonObject gameStateData;
    gameStateData["currentChapter"] = gameState->getCurrentChapter();
    gameStateData["currentScene"] = gameState->getCurrentScene();
    gameStateData["tutorialComplete"] = gameState->isTutorialComplete();
    gameStateData["gameProgress"] = gameState->getGameProgress();
    
    // 保存世界变量
    QJsonObject worldVars;
    const auto &variables = gameState->getWorldVariables();
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        worldVars[it.key()] = it.value();
    }
    gameStateData["worldVariables"] = worldVars;
    
    // 保存世界标志
    QJsonObject worldFlags;
    const auto &flags = gameState->getWorldFlags();
    for (auto it = flags.begin(); it != flags.end(); ++it) {
        worldFlags[it.key()] = it.value();
    }
    gameStateData["worldFlags"] = worldFlags;
    
    // 保存完成的任务
    QJsonArray completedQuests;
    for (const QString &quest : gameState->getCompletedQuests()) {
        completedQuests.append(quest);
    }
    gameStateData["completedQuests"] = completedQuests;
    
    // 保存活跃任务
    QJsonArray activeQuests;
    for (const QString &quest : gameState->getActiveQuests()) {
        activeQuests.append(quest);
    }
    gameStateData["activeQuests"] = activeQuests;
    
    saveData["gameState"] = gameStateData;
    
    // 写入文件
    QString savePath = getSavePath(slot);
    QFile file(savePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SaveManager: 无法创建存档文件:" << savePath;
        return false;
    }
    
    QJsonDocument doc(saveData);
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "SaveManager: 游戏保存成功，槽位:" << slot;
    emit gameSaved(slot);
    return true;
}

bool SaveManager::loadGame(int slot, Player *player, GameState *gameState)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        qWarning() << "SaveManager: 无效的存档槽位:" << slot;
        return false;
    }
    
    if (!player || !gameState) {
        qWarning() << "SaveManager: 玩家或游戏状态为空";
        return false;
    }
    
    QString savePath = getSavePath(slot);
    QFile file(savePath);
    
    if (!file.exists()) {
        qWarning() << "SaveManager: 存档文件不存在:" << savePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SaveManager: 无法打开存档文件:" << savePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "SaveManager: 存档文件JSON解析错误:" << error.errorString();
        return false;
    }
    
    QJsonObject saveData = doc.object();
    
    // 验证存档版本
    QJsonObject metadata = saveData["metadata"].toObject();
    QString version = metadata["version"].toString();
    if (version != "1.0.0") {
        qWarning() << "SaveManager: 不支持的存档版本:" << version;
        return false;
    }
    
    // 加载玩家数据
    QJsonObject playerData = saveData["player"].toObject();
    player->setName(playerData["name"].toString());
    player->setLevel(playerData["level"].toInt());
    player->setExperience(playerData["experience"].toInt());
    player->setProfession(static_cast<Player::Profession>(playerData["profession"].toInt()));
    player->setHealth(playerData["health"].toInt());
    player->setMaxHealth(playerData["maxHealth"].toInt());
    player->setMana(playerData["mana"].toInt());
    player->setMaxMana(playerData["maxMana"].toInt());
    player->setAttack(playerData["attack"].toInt());
    player->setDefense(playerData["defense"].toInt());
    player->setSpeed(playerData["speed"].toInt());
    player->setLuck(playerData["luck"].toInt());
    
    // 加载状态效果
    QJsonArray statusEffects = playerData["statusEffects"].toArray();
    QList<Player::StatusEffect> effects;
    for (const QJsonValue &value : statusEffects) {
        QJsonObject effectObj = value.toObject();
        Player::StatusEffect effect;
        effect.type = static_cast<Player::StatusEffectType>(effectObj["type"].toInt());
        effect.duration = effectObj["duration"].toInt();
        effect.value = effectObj["value"].toDouble();
        effects.append(effect);
    }
    player->setStatusEffects(effects);
    
    // 加载游戏状态
    QJsonObject gameStateData = saveData["gameState"].toObject();
    gameState->setCurrentChapter(gameStateData["currentChapter"].toInt());
    gameState->setCurrentScene(gameStateData["currentScene"].toString());
    gameState->setTutorialComplete(gameStateData["tutorialComplete"].toBool());
    
    // 加载世界变量
    QJsonObject worldVars = gameStateData["worldVariables"].toObject();
    QMap<QString, int> variables;
    for (auto it = worldVars.begin(); it != worldVars.end(); ++it) {
        variables[it.key()] = it.value().toInt();
    }
    gameState->setWorldVariables(variables);
    
    // 加载世界标志
    QJsonObject worldFlags = gameStateData["worldFlags"].toObject();
    QMap<QString, bool> flags;
    for (auto it = worldFlags.begin(); it != worldFlags.end(); ++it) {
        flags[it.key()] = it.value().toBool();
    }
    gameState->setWorldFlags(flags);
    
    // 加载完成的任务
    QJsonArray completedQuests = gameStateData["completedQuests"].toArray();
    QStringList completed;
    for (const QJsonValue &value : completedQuests) {
        completed.append(value.toString());
    }
    gameState->setCompletedQuests(completed);
    
    // 加载活跃任务
    QJsonArray activeQuests = gameStateData["activeQuests"].toArray();
    QStringList active;
    for (const QJsonValue &value : activeQuests) {
        active.append(value.toString());
    }
    gameState->setActiveQuests(active);
    
    qDebug() << "SaveManager: 游戏加载成功，槽位:" << slot;
    emit gameLoaded(slot);
    return true;
}

bool SaveManager::deleteSave(int slot)
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        qWarning() << "SaveManager: 无效的存档槽位:" << slot;
        return false;
    }
    
    QString savePath = getSavePath(slot);
    QFile file(savePath);
    
    if (!file.exists()) {
        qDebug() << "SaveManager: 存档文件不存在，无需删除:" << savePath;
        return true;
    }
    
    bool success = file.remove();
    if (success) {
        qDebug() << "SaveManager: 存档删除成功，槽位:" << slot;
        emit saveDeleted(slot);
    } else {
        qWarning() << "SaveManager: 存档删除失败，槽位:" << slot;
    }
    
    return success;
}

bool SaveManager::hasSave(int slot) const
{
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        return false;
    }
    
    QString savePath = getSavePath(slot);
    return QFile::exists(savePath);
}

SaveInfo SaveManager::getSaveInfo(int slot) const
{
    SaveInfo info;
    info.slot = slot;
    info.exists = false;
    
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        return info;
    }
    
    QString savePath = getSavePath(slot);
    QFile file(savePath);
    
    if (!file.exists()) {
        return info;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return info;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return info;
    }
    
    QJsonObject saveData = doc.object();
    QJsonObject metadata = saveData["metadata"].toObject();
    QJsonObject playerData = saveData["player"].toObject();
    QJsonObject gameStateData = saveData["gameState"].toObject();
    
    info.exists = true;
    info.timestamp = QDateTime::fromString(metadata["timestamp"].toString(), Qt::ISODate);
    info.playerName = playerData["name"].toString();
    info.playerLevel = playerData["level"].toInt();
    info.profession = static_cast<Player::Profession>(playerData["profession"].toInt());
    info.currentChapter = gameStateData["currentChapter"].toInt();
    info.gameProgress = gameStateData["gameProgress"].toInt();
    
    return info;
}

QList<SaveInfo> SaveManager::getAllSaveInfo() const
{
    QList<SaveInfo> saveList;
    
    for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
        saveList.append(getSaveInfo(i));
    }
    
    return saveList;
}

void SaveManager::initializeSaveDirectory()
{
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    saveDir += "/FantasyLegend/Saves";
    
    QDir dir;
    if (!dir.exists(saveDir)) {
        dir.mkpath(saveDir);
        qDebug() << "SaveManager: 创建存档目录:" << saveDir;
    }
}

QString SaveManager::getSavePath(int slot) const
{
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    saveDir += "/FantasyLegend/Saves";
    return QString("%1/save_%2.json").arg(saveDir).arg(slot);
}