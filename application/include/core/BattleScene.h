/*
 * 文件名: BattleScene.h
 * 说明: 战斗场景
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef APPLICATION_CORE_BATTLESCENE_H
#define APPLICATION_CORE_BATTLESCENE_H

#include "core/Scene.h"

class BattleScene : public Scene {
    Q_OBJECT

public:
    explicit BattleScene(QObject* parent = nullptr);
    ~BattleScene() override = default;

    bool load(ResourceManager* resourceManager) override;
    void onEnter(const QJsonObject& sceneData) override;
    void update(float deltaTime) override;
    void onExit() override;
    void unload() override;
};

#endif // APPLICATION_CORE_BATTLESCENE_H
