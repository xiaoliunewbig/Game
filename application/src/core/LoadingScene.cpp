/*
 * 文件名: LoadingScene.cpp
 * 说明: 加载场景实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "core/LoadingScene.h"
#include <QDebug>

LoadingScene::LoadingScene(QObject* parent)
    : Scene("Loading", parent) {
}

bool LoadingScene::load(ResourceManager* resourceManager) {
    Q_UNUSED(resourceManager)
    qDebug() << "LoadingScene: 加载资源";
    return true;
}

void LoadingScene::onEnter(const QJsonObject& sceneData) {
    Q_UNUSED(sceneData)
    qDebug() << "LoadingScene: 进入场景";
}

void LoadingScene::update(float deltaTime) {
    Q_UNUSED(deltaTime)
}

void LoadingScene::onExit() {
    qDebug() << "LoadingScene: 退出场景";
}

void LoadingScene::unload() {
    qDebug() << "LoadingScene: 卸载资源";
}
