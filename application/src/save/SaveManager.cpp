/*
 * 文件名: SaveManager.cpp
 * 说明: 存档管理器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
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

    // 使用Player自带的序列化
    saveData["player"] = player->toJson();

    // 使用GameState自带的序列化
    saveData["gameState"] = gameState->toJson();

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

    // 使用Player自带的反序列化
    player->loadFromJson(saveData["player"].toObject());

    // 使用GameState自带的反序列化
    gameState->loadFromJson(saveData["gameState"].toObject());

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
    info.profession = static_cast<PlayerProfession>(playerData["profession"].toInt());
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