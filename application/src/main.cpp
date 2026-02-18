/*
 * 文件名: main.cpp
 * 说明: 应用程序入口点
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 版本: v1.0.0
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QPalette>
#include <QColor>
#include "GameApplication.h"
#include <QFontDatabase>
#include "utils/Logger.h"

/**
 * @brief 设置应用程序基本信息
 */
void setupApplicationInfo()
{
    QCoreApplication::setApplicationName("幻境传说");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("Game Studio");
    QCoreApplication::setOrganizationDomain("gamestudio.com");
}

/**
 * @brief 设置应用程序样式和主题
 */
void setupApplicationStyle(QApplication& app)
{
    // 设置应用程序样式为深色主题
    app.setStyle("Fusion");
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
}

/**
 * @brief 初始化日志系统
 */
void initializeLogging()
{
    // 创建日志目录
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);

    // 初始化日志系统
    Logger* logger = Logger::instance();
    if (logger) {
        logger->setLogLevel(LogLevel::Debug);
        logger->setFileOutput(true);
        logger->setConsoleOutput(true);
    }

    qDebug() << "日志系统初始化完成，日志目录:" << logDir;
}

/**
 * @brief 加载自定义字体
 */
void loadCustomFonts()
{
    // 加载游戏字体
    QString fontPath = ":/resources/fonts/game_font.ttf";
    if (QFile::exists(fontPath)) {
        int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (!fontFamilies.isEmpty()) {
                qDebug() << "成功加载自定义字体:" << fontFamilies.first();
            }
        } else {
            qDebug() << "自定义字体加载失败，使用系统默认字体:" << fontPath;
        }
    } else {
        qDebug() << "自定义字体文件不存在，使用系统默认字体:" << fontPath;
    }
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[])
{
    // 创建应用程序实例
    QApplication app(argc, argv);
    
    // 设置应用程序基本信息
    setupApplicationInfo();
    
    // 初始化日志系统
    initializeLogging();
    
    // 设置应用程序样式
    setupApplicationStyle(app);
    
    // 加载自定义字体
    loadCustomFonts();
    
    try {
        // 创建游戏应用程序
        GameApplication gameApp;
        
        // 初始化游戏应用程序
        if (!gameApp.initialize()) {
            qCritical() << "游戏应用程序初始化失败";
            return -1;
        }
        
        // 启动游戏应用程序
        gameApp.start();
        
        qDebug() << "游戏应用程序启动成功";
        
        // 进入事件循环
        int result = app.exec();
        
        // 关闭游戏应用程序
        gameApp.shutdown();
        
        qDebug() << "应用程序正常退出，返回码:" << result;
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "应用程序异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "应用程序发生未知异常";
        return -1;
    }
}
