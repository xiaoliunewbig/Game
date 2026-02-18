/*
 * 文件名: Scene.h
 * 说明: 场景抽象基类，定义场景生命周期接口
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */
#ifndef APPLICATION_CORE_SCENE_H
#define APPLICATION_CORE_SCENE_H

#include <QObject>
#include <QJsonObject>
#include <QString>

class ResourceManager;

/**
 * @brief 场景抽象基类
 *
 * 定义场景的生命周期方法，所有具体场景必须继承并实现此类。
 */
class Scene : public QObject {
    Q_OBJECT

public:
    explicit Scene(const QString& name, QObject* parent = nullptr)
        : QObject(parent), name_(name) {}

    virtual ~Scene() = default;

    /**
     * @brief 加载场景资源
     * @param resourceManager 资源管理器
     * @return 是否加载成功
     */
    virtual bool load(ResourceManager* resourceManager) = 0;

    /**
     * @brief 场景进入时调用
     * @param sceneData 传入场景的数据
     */
    virtual void onEnter(const QJsonObject& sceneData) = 0;

    /**
     * @brief 场景更新
     * @param deltaTime 帧间隔时间（秒）
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief 场景退出时调用
     */
    virtual void onExit() = 0;

    /**
     * @brief 卸载场景资源
     */
    virtual void unload() = 0;

    /**
     * @brief 获取场景名称
     */
    const QString& name() const { return name_; }

protected:
    QString name_;
};

#endif // APPLICATION_CORE_SCENE_H
