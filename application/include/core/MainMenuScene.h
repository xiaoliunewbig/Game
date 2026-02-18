/*
 * 文件名: MainMenuScene.h
 * 说明: 主菜单场景
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef APPLICATION_CORE_MAINMENUSCENE_H
#define APPLICATION_CORE_MAINMENUSCENE_H

#include "core/Scene.h"

class MainMenuScene : public Scene {
    Q_OBJECT

public:
    explicit MainMenuScene(QObject* parent = nullptr);
    ~MainMenuScene() override = default;

    bool load(ResourceManager* resourceManager) override;
    void onEnter(const QJsonObject& sceneData) override;
    void update(float deltaTime) override;
    void onExit() override;
    void unload() override;
};

#endif // APPLICATION_CORE_MAINMENUSCENE_H
