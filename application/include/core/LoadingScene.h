/*
 * 文件名: LoadingScene.h
 * 说明: 加载场景
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef APPLICATION_CORE_LOADINGSCENE_H
#define APPLICATION_CORE_LOADINGSCENE_H

#include "core/Scene.h"

class LoadingScene : public Scene {
    Q_OBJECT

public:
    explicit LoadingScene(QObject* parent = nullptr);
    ~LoadingScene() override = default;

    bool load(ResourceManager* resourceManager) override;
    void onEnter(const QJsonObject& sceneData) override;
    void update(float deltaTime) override;
    void onExit() override;
    void unload() override;
};

#endif // APPLICATION_CORE_LOADINGSCENE_H
