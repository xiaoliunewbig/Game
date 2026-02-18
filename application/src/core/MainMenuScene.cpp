/*
 * 文件名: MainMenuScene.cpp
 * 说明: 主菜单场景实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "core/MainMenuScene.h"
#include <QDebug>

MainMenuScene::MainMenuScene(QObject* parent)
    : Scene("MainMenu", parent) {
}

bool MainMenuScene::load(ResourceManager* resourceManager) {
    Q_UNUSED(resourceManager)
    qDebug() << "MainMenuScene: 加载资源";
    return true;
}

void MainMenuScene::onEnter(const QJsonObject& sceneData) {
    Q_UNUSED(sceneData)
    qDebug() << "MainMenuScene: 进入场景";
}

void MainMenuScene::update(float deltaTime) {
    Q_UNUSED(deltaTime)
}

void MainMenuScene::onExit() {
    qDebug() << "MainMenuScene: 退出场景";
}

void MainMenuScene::unload() {
    qDebug() << "MainMenuScene: 卸载资源";
}
