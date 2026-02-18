/*
 * 文件名: MainMenu.qml
 * 说明: 游戏主菜单界面
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import GameUI 1.0
import "components"

/**
 * @brief 游戏主菜单
 * 
 * 提供游戏的主要入口功能：
 * - 新游戏
 * - 加载游戏
 * - 设置
 * - 退出游戏
 */
Rectangle {
    id: mainMenu
    
    // 信号定义
    signal newGameClicked(string playerName, int profession)
    signal loadGameClicked(int saveSlot)
    signal settingsClicked()
    signal exitClicked()
    
    // 背景
    color: "#1a1a1a"
    
    // 背景图片
    Image {
        id: backgroundImage
        anchors.fill: parent
        source: "qrc:/resources/images/main_menu_bg.jpg"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.7
    }
    
    // 渐变遮罩
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: "#80000000" }
        }
    }
    
    // 主要内容区域
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30
        
        // 游戏标题
        Text {
            id: gameTitle
            text: "幻境传说"
            font.family: "Microsoft YaHei"
            font.pixelSize: 72
            font.bold: true
            color: "#FFD700"
            Layout.alignment: Qt.AlignHCenter
            
            // 文字阴影效果（简化版）
            style: Text.Outline
            styleColor: "#80FFD700"
        }
        
        // 副标题
        Text {
            text: "Fantasy Legend"
            font.family: "Arial"
            font.pixelSize: 24
            font.italic: true
            color: "#CCCCCC"
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 菜单按钮区域
        ColumnLayout {
            spacing: 15
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 50
            
            // 新游戏按钮
            MenuButton {
                id: newGameButton
                text: "新游戏"
                iconSource: "qrc:/resources/icons/new_game.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    newGameDialog.open()
                }
            }
            
            // 加载游戏按钮
            MenuButton {
                id: loadGameButton
                text: "加载游戏"
                iconSource: "qrc:/resources/icons/load_game.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    loadGameDialog.open()
                }
            }
            
            // 设置按钮
            MenuButton {
                id: settingsButton
                text: "设置"
                iconSource: "qrc:/resources/icons/settings.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    mainMenu.settingsClicked()
                }
            }
            
            // 退出游戏按钮
            MenuButton {
                id: exitButton
                text: "退出游戏"
                iconSource: "qrc:/resources/icons/exit.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    exitConfirmDialog.open()
                }
            }
        }
    }
    
    // 版本信息
    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        text: "版本 1.0.0"
        font.pixelSize: 14
        color: "#888888"
    }
    
    // 新游戏对话框
    Dialog {
        id: newGameDialog
        title: "创建新角色"
        modal: true
        anchors.centerIn: parent
        width: 500
        height: 600
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 2
            radius: 10
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            // 角色名称输入
            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true

                Text {
                    text: "角色名称："
                    color: "#FFFFFF"
                    font.pixelSize: 16
                }
                
                TextField {
                    id: playerNameField
                    Layout.fillWidth: true
                    placeholderText: "请输入角色名称"
                    font.pixelSize: 14
                    maximumLength: 20
                    
                    background: Rectangle {
                        color: "#3a3a3a"
                        border.color: "#555555"
                        border.width: 1
                        radius: 5
                    }
                }
            }
            
            // 职业选择
            ColumnLayout {
                spacing: 15
                Layout.fillWidth: true

                Text {
                    text: "选择职业："
                    color: "#FFFFFF"
                    font.pixelSize: 16
                }
                
                Row {
                    spacing: 20
                    Layout.alignment: Qt.AlignHCenter
                    
                    ProfessionCard {
                        id: warriorCard
                        profession: 0 // Warrior
                        professionName: "战士"
                        description: "近战物理攻击专家\n高血量和防御力"
                        iconSource: "qrc:/resources/icons/warrior.png"
                        selected: true
                    }
                    
                    ProfessionCard {
                        id: mageCard
                        profession: 1 // Mage
                        professionName: "法师"
                        description: "远程魔法攻击专家\n高魔法攻击力"
                        iconSource: "qrc:/resources/icons/mage.png"
                    }
                    
                    ProfessionCard {
                        id: archerCard
                        profession: 2 // Archer
                        professionName: "弓箭手"
                        description: "远程物理攻击专家\n高敏捷和暴击率"
                        iconSource: "qrc:/resources/icons/archer.png"
                    }
                }
            }
            
            // 按钮区域
            RowLayout {
                Layout.topMargin: 20
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: "开始游戏"
                    enabled: playerNameField.text.length > 0
                    
                    onClicked: {
                        let selectedProfession = 0
                        if (mageCard.selected) selectedProfession = 1
                        else if (archerCard.selected) selectedProfession = 2
                        
                        mainMenu.newGameClicked(playerNameField.text, selectedProfession)
                        newGameDialog.close()
                    }
                }
                
                Button {
                    text: "取消"
                    onClicked: newGameDialog.close()
                }
            }
        }
    }
    
    // 加载游戏对话框
    Dialog {
        id: loadGameDialog
        title: "加载游戏"
        modal: true
        anchors.centerIn: parent
        width: 600
        height: 500
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 2
            radius: 10
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            Text {
                text: "选择存档："
                color: "#FFFFFF"
                font.pixelSize: 16
            }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                ListView {
                    id: saveListView
                    model: 10 // 10个存档槽位
                    spacing: 10
                    
                    delegate: SaveSlotItem {
                        width: saveListView.width
                        slotIndex: index
                        
                        onLoadRequested: function(slotIndex) {
                            mainMenu.loadGameClicked(slotIndex)
                            loadGameDialog.close()
                        }
                    }
                }
            }
            
            Button {
                text: "取消"
                Layout.alignment: Qt.AlignHCenter
                onClicked: loadGameDialog.close()
            }
        }
    }
    
    // 退出确认对话框
    Dialog {
        id: exitConfirmDialog
        title: "确认退出"
        modal: true
        anchors.centerIn: parent
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 2
            radius: 10
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Text {
                text: "确定要退出游戏吗？"
                color: "#FFFFFF"
                font.pixelSize: 16
                Layout.alignment: Qt.AlignHCenter
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: "确定"
                    onClicked: {
                        mainMenu.exitClicked()
                        exitConfirmDialog.close()
                    }
                }
                
                Button {
                    text: "取消"
                    onClicked: exitConfirmDialog.close()
                }
            }
        }
    }
}
