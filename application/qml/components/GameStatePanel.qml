/*
 * 文件名: GameStatePanel.qml
 * 说明: 游戏状态面板组件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 游戏状态面板
 * 
 * 显示当前游戏状态信息：
 * - 当前章节和场景
 * - 游戏时间
 * - 任务进度
 * - 世界状态
 */
Rectangle {
    id: gameStatePanel
    
    color: "#3a3a3a"
    border.color: "#555555"
    border.width: 1
    radius: 8
    
    // 游戏状态数据绑定（从GameState C++对象获取）
    property int currentChapter: 1
    property string currentScene: gameState ? gameState.currentScene : "未知"
    property string gameTime: "00:00:00"
    property int activeQuests: 3
    property int completedQuests: 7
    property bool inCombat: false
    property int playerGold: gameState ? gameState.playerGold : 0
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10
        
        // 标题
        Text {
            text: "游戏状态"
            font.family: "Microsoft YaHei"
            font.pixelSize: 14
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#555555"
        }
        
        // 章节信息
        RowLayout {
            Layout.fillWidth: true
            
            Image {
                source: "qrc:/resources/icons/chapter.png"
                width: 16
                height: 16
            }
            
            Text {
                text: "章节:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: `第${currentChapter}章`
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }
        
        // 当前场景
        RowLayout {
            Layout.fillWidth: true
            
            Image {
                source: "qrc:/resources/icons/location.png"
                width: 16
                height: 16
            }
            
            Text {
                text: "位置:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: currentScene
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }
        
        // 游戏时间
        RowLayout {
            Layout.fillWidth: true
            
            Image {
                source: "qrc:/resources/icons/time.png"
                width: 16
                height: 16
            }
            
            Text {
                text: "时间:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: gameTime
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }
        
        // 任务进度
        RowLayout {
            Layout.fillWidth: true
            
            Image {
                source: "qrc:/resources/icons/quest.png"
                width: 16
                height: 16
            }
            
            Text {
                text: "任务:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: `${activeQuests}进行 / ${completedQuests}完成`
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }
        
        // 战斗状态指示器
        Rectangle {
            Layout.fillWidth: true
            height: 25
            color: inCombat ? "#FF5722" : "#4CAF50"
            radius: 4
            border.color: inCombat ? "#D32F2F" : "#388E3C"
            border.width: 1
            
            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#FFFFFF"
                    
                    // 闪烁动画（战斗状态时）
                    SequentialAnimation on opacity {
                        running: inCombat
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }
                
                Text {
                    text: inCombat ? "战斗中" : "探索中"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#FFFFFF"
                }
            }
        }
    }
    
    // 定时器更新游戏时间
    Timer {
        id: gameTimeTimer
        interval: 1000
        running: true
        repeat: true
        
        onTriggered: {
            // 这里应该从GameEngine获取实际游戏时间
            // 现在只是模拟时间更新
            updateGameTime()
        }
    }
    
    // 更新游戏时间的函数
    function updateGameTime() {
        let time = gameTime.split(":")
        let hours = parseInt(time[0])
        let minutes = parseInt(time[1])
        let seconds = parseInt(time[2])
        
        seconds++
        if (seconds >= 60) {
            seconds = 0
            minutes++
            if (minutes >= 60) {
                minutes = 0
                hours++
            }
        }
        
        gameTime = String(hours).padStart(2, '0') + ":" +
                   String(minutes).padStart(2, '0') + ":" +
                   String(seconds).padStart(2, '0')
    }
}
