/*
 * 文件名: GameplayScene.cpp
 * 说明: 游戏主场景实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "core/GameplayScene.h"
#include <QDebug>

GameplayScene::GameplayScene(QObject* parent)
    : Scene("Gameplay", parent) {
}

bool GameplayScene::load(ResourceManager* resourceManager) {
    Q_UNUSED(resourceManager)
    qDebug() << "GameplayScene: 加载资源";
    return true;
}

void GameplayScene::onEnter(const QJsonObject& sceneData) {
    Q_UNUSED(sceneData)
    qDebug() << "GameplayScene: 进入场景";
}

void GameplayScene::update(float deltaTime) {
    Q_UNUSED(deltaTime)
}

void GameplayScene::onExit() {
    qDebug() << "GameplayScene: 退出场景";
}

void GameplayScene::unload() {
    qDebug() << "GameplayScene: 卸载资源";
}
