/*
 * 文件名: SaveManager.h
 * 说明: 存档管理器头文件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include "game/Player.h"
#include "game/GameState.h"

/**
 * @brief 存档信息结构体
 */
struct SaveInfo
{
    int slot;                           // 存档槽位
    bool exists;                        // 是否存在存档
    QDateTime timestamp;                // 保存时间
    QString playerName;                 // 玩家名称
    int playerLevel;                    // 玩家等级
    PlayerProfession profession;        // 玩家职业
    int currentChapter;                 // 当前章节
    int gameProgress;                   // 游戏进度百分比
};

/**
 * @brief 存档管理器
 * 
 * 负责游戏存档的保存、加载和管理，包括：
 * - 玩家数据的序列化和反序列化
 * - 游戏状态的持久化
 * - 存档文件的管理
 * - 存档信息的查询
 */
class SaveManager : public QObject
{
    Q_OBJECT

public:
    static const int MAX_SAVE_SLOTS = 10;  // 最大存档槽位数

    explicit SaveManager(QObject *parent = nullptr);

    /**
     * @brief 保存游戏
     * 
     * @param slot 存档槽位 (0-9)
     * @param player 玩家对象
     * @param gameState 游戏状态对象
     * @return bool 保存是否成功
     */
    bool saveGame(int slot, const Player *player, const GameState *gameState);

    /**
     * @brief 加载游戏
     * 
     * @param slot 存档槽位 (0-9)
     * @param player 玩家对象（用于接收数据）
     * @param gameState 游戏状态对象（用于接收数据）
     * @return bool 加载是否成功
     */
    bool loadGame(int slot, Player *player, GameState *gameState);

    /**
     * @brief 删除存档
     * 
     * @param slot 存档槽位 (0-9)
     * @return bool 删除是否成功
     */
    bool deleteSave(int slot);

    /**
     * @brief 检查存档是否存在
     * 
     * @param slot 存档槽位 (0-9)
     * @return bool 存档是否存在
     */
    bool hasSave(int slot) const;

    /**
     * @brief 获取存档信息
     * 
     * @param slot 存档槽位 (0-9)
     * @return SaveInfo 存档信息
     */
    SaveInfo getSaveInfo(int slot) const;

    /**
     * @brief 获取所有存档信息
     * 
     * @return QList<SaveInfo> 所有存档信息列表
     */
    QList<SaveInfo> getAllSaveInfo() const;

signals:
    /**
     * @brief 游戏保存完成信号
     * 
     * @param slot 存档槽位
     */
    void gameSaved(int slot);

    /**
     * @brief 游戏加载完成信号
     * 
     * @param slot 存档槽位
     */
    void gameLoaded(int slot);

    /**
     * @brief 存档删除完成信号
     * 
     * @param slot 存档槽位
     */
    void saveDeleted(int slot);

private:
    /**
     * @brief 初始化存档目录
     */
    void initializeSaveDirectory();

    /**
     * @brief 获取存档文件路径
     * 
     * @param slot 存档槽位
     * @return QString 存档文件路径
     */
    QString getSavePath(int slot) const;
};

// 注册SaveInfo为Qt元类型，以便在QML中使用
Q_DECLARE_METATYPE(SaveInfo)

#endif // SAVEMANAGER_H