/*
 * 文件名: GameplayScene.h
 * 说明: 游戏主场景
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef APPLICATION_CORE_GAMEPLAYSCENE_H
#define APPLICATION_CORE_GAMEPLAYSCENE_H

#include "core/Scene.h"

class GameplayScene : public Scene {
    Q_OBJECT

public:
    explicit GameplayScene(QObject* parent = nullptr);
    ~GameplayScene() override = default;

    bool load(ResourceManager* resourceManager) override;
    void onEnter(const QJsonObject& sceneData) override;
    void update(float deltaTime) override;
    void onExit() override;
    void unload() override;
};

#endif // APPLICATION_CORE_GAMEPLAYSCENE_H
