/*
 * 文件名: main.cpp
 * 说明: 游戏应用程序的主入口点。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件包含应用程序的main函数，负责创建QApplication实例，
 * 初始化游戏应用程序，并启动主事件循环。提供应用程序的
 * 基础配置和错误处理机制。
 */
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>

#include "GameApplication.h"
#include "core/GameEngine.h"

/**
 * @brief 应用程序主入口函数
 * 
 * 创建并配置Qt应用程序，初始化游戏系统，启动主事件循环。
 * 
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 应用程序退出码
 * @retval 0 正常退出
 * @retval -1 初始化失败
 * @retval 其他 异常退出
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序实例
    QApplication app(argc, argv);
    
    // 设置应用程序基本信息
    app.setApplicationName("幻境传说");           // 应用程序名称
    app.setApplicationVersion("1.0.0");          // 版本号
    app.setOrganizationName("Game Studio");      // 组织名称
    app.setOrganizationDomain("gamestudio.com"); // 组织域名
    
    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/resources/images/app_icon.png"));
    
    // 配置日志系统
    QLoggingCategory::setFilterRules("qt.qml.debug=false");
    
    // 设置UI样式主题
    QQuickStyle::setStyle("Material");
    
    // 确保应用程序数据目录存在
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    // 创建游戏应用程序实例
    GameApplication gameApp;
    
    // 初始化游戏应用程序
    if (!gameApp.initialize()) {
        qCritical() << "游戏应用程序初始化失败";
        return -1;
    }
    
    // 启动游戏应用程序
    gameApp.start();
    
    // 进入主事件循环
    int result = app.exec();
    
    // 应用程序退出时执行清理
    gameApp.shutdown();
    
    return result;
}
