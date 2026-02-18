/*
 * 文件名: BattleScene.cpp
 * 说明: 战斗场景实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#include "core/BattleScene.h"
#include <QDebug>

BattleScene::BattleScene(QObject* parent)
    : Scene("Battle", parent) {
}

bool BattleScene::load(ResourceManager* resourceManager) {
    Q_UNUSED(resourceManager)
    qDebug() << "BattleScene: 加载资源";
    return true;
}

void BattleScene::onEnter(const QJsonObject& sceneData) {
    Q_UNUSED(sceneData)
    qDebug() << "BattleScene: 进入场景";
}

void BattleScene::update(float deltaTime) {
    Q_UNUSED(deltaTime)
}

void BattleScene::onExit() {
    qDebug() << "BattleScene: 退出场景";
}

void BattleScene::unload() {
    qDebug() << "BattleScene: 卸载资源";
}
