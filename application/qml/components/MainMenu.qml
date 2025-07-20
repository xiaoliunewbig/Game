/*
 * 文件名: MainMenu.qml
 * 说明: 游戏主菜单界面组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本组件实现游戏的主菜单界面，包括新游戏、加载游戏、
 * 设置和退出等功能。
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

/**
 * @brief 主菜单组件
 * 
 * 提供游戏的主要入口界面，包括：
 * - 新游戏选项
 * - 加载游戏选项
 * - 游戏设置
 * - 退出游戏
 */
Rectangle {
    id: root
    
    // 组件属性
    property bool animationsEnabled: true
    property string selectedProfession: "warrior"
    property string playerName: ""
    
    // 信号定义
    signal newGameClicked(string playerName, string profession)
    signal loadGameClicked(int saveSlot)
    signal settingsClicked()
    signal exitClicked()
    
    // 背景设置
    color: "#1a1a2e"
    
    // 背景图片
    Image {
        id: backgroundImage
        anchors.fill: parent
        source: "qrc:/resources/images/main_menu_bg.jpg"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.7
        
        // 背景动画效果
        SequentialAnimation on opacity {
            running: root.animationsEnabled
            loops: Animation.Infinite
            NumberAnimation { to: 0.5; duration: 3000 }
            NumberAnimation { to: 0.7; duration: 3000 }
        }
    }
    
    // 主要内容区域
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30
        width: Math.min(parent.width * 0.8, 600)
        
        // 游戏标题
        Text {
            id: gameTitle
            text: "幻境传说"
            font.family: "Microsoft YaHei"
            font.pixelSize: 72
            font.bold: true
            color: "#ffffff"
            Layout.alignment: Qt.AlignHCenter
            
            // 标题发光效果
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 0
                radius: 20
                samples: 41
                color: "#4a90e2"
                spread: 0.3
            }
            
            // 标题动画
            NumberAnimation on scale {
                running: root.animationsEnabled
                from: 0.95
                to: 1.05
                duration: 2000
                loops: Animation.Infinite
                easing.type: Easing.InOutSine
            }
        }
        
        // 版本信息
        Text {
            text: "v1.0.0 Alpha"
            font.family: "Microsoft YaHei"
            font.pixelSize: 16
            color: "#cccccc"
            Layout.alignment: Qt.AlignHCenter
            opacity: 0.8
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
                icon: "qrc:/resources/icons/new_game.png"
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
                icon: "qrc:/resources/icons/load_game.png"
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
                icon: "qrc:/resources/icons/settings.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    root.settingsClicked()
                }
            }
            
            // 退出游戏按钮
            MenuButton {
                id: exitButton
                text: "退出游戏"
                icon: "qrc:/resources/icons/exit.png"
                Layout.preferredWidth: 300
                Layout.preferredHeight: 60
                
                onClicked: {
                    exitConfirmDialog.open()
                }
            }
        }
    }
    
    // 新游戏对话框
    Dialog {
        id: newGameDialog
        title: "创建新角色"
        modal: true
        anchors.centerIn: parent
        width: 500
        height: 400
        
        background: Rectangle {
            color: "#2d2d44"
            radius: 10
            border.color: "#4a90e2"
            border.width: 2
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            // 角色名称输入
            ColumnLayout {
                spacing: 10
                
                Text {
                    text: "角色名称:"
                    color: "#ffffff"
                    font.pixelSize: 16
                }
                
                TextField {
                    id: nameInput
                    placeholderText: "请输入角色名称"
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    
                    background: Rectangle {
                        color: "#1a1a2e"
                        border.color: nameInput.focus ? "#4a90e2" : "#666666"
                        border.width: 2
                        radius: 5
                    }
                    
                    color: "#ffffff"
                    
                    onTextChanged: {
                        root.playerName = text
                    }
                }
            }
            
            // 职业选择
            ColumnLayout {
                spacing: 10
                
                Text {
                    text: "选择职业:"
                    color: "#ffffff"
                    font.pixelSize: 16
                }
                
                Row {
                    spacing: 20
                    Layout.alignment: Qt.AlignHCenter
                    
                    ProfessionCard {
                        profession: "warrior"
                        name: "战士"
                        description: "近战专家，高血量高防御"
                        icon: "qrc:/resources/icons/warrior.png"
                        selected: root.selectedProfession === "warrior"
                        
                        onClicked: {
                            root.selectedProfession = "warrior"
                        }
                    }
                    
                    ProfessionCard {
                        profession: "mage"
                        name: "法师"
                        description: "魔法专家，高伤害远程攻击"
                        icon: "qrc:/resources/icons/mage.png"
                        selected: root.selectedProfession === "mage"
                        
                        onClicked: {
                            root.selectedProfession = "mage"
                        }
                    }
                    
                    ProfessionCard {
                        profession: "archer"
                        name: "弓箭手"
                        description: "敏捷专家，高速度高暴击"
                        icon: "qrc:/resources/icons/archer.png"
                        selected: root.selectedProfession === "archer"
                        
                        onClicked: {
                            root.selectedProfession = "archer"
                        }
                    }
                }
            }
            
            // 按钮区域
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: "开始游戏"
                    enabled: root.playerName.length > 0
                    
                    background: Rectangle {
                        color: parent.enabled ? "#4a90e2" : "#666666"
                        radius: 5
                    }
                    
                    onClicked: {
                        newGameDialog.close()
                        root.newGameClicked(root.playerName, root.selectedProfession)
                    }
                }
                
                Button {
                    text: "取消"
                    
                    background: Rectangle {
                        color: "#666666"
                        radius: 5
                    }
                    
                    onClicked: {
                        newGameDialog.close()
                    }
                }
            }
        }
    }
    
    // 加载游戏对话框
    Dialog {
        id: loadGameDialog
        title: "加载存档"
        modal: true
        anchors.centerIn: parent
        width: 600
        height: 500
        
        background: Rectangle {
            color: "#2d2d44"
            radius: 10
            border.color: "#4a90e2"
            border.width: 2
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Text {
                text: "选择存档槽位:"
                color: "#ffffff"
                font.pixelSize: 16
            }
            
            // 存档列表
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                ListView {
                    id: savesList
                    model: 5 // 5个存档槽位
                    
                    delegate: SaveSlotItem {
                        width: savesList.width
                        slotIndex: index
                        
                        onLoadClicked: {
                            loadGameDialog.close()
                            root.loadGameClicked(slotIndex)
                        }
                    }
                }
            }
            
            // 关闭按钮
            Button {
                text: "关闭"
                Layout.alignment: Qt.AlignHCenter
                
                background: Rectangle {
                    color: "#666666"
                    radius: 5
                }
                
                onClicked: {
                    loadGameDialog.close()
                }
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
            color: "#2d2d44"
            radius: 10
            border.color: "#e74c3c"
            border.width: 2
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Text {
                text: "确定要退出游戏吗？"
                color: "#ffffff"
                font.pixelSize: 16
                Layout.alignment: Qt.AlignHCenter
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: "确定"
                    
                    background: Rectangle {
                        color: "#e74c3c"
                        radius: 5
                    }
                    
                    onClicked: {
                        exitConfirmDialog.close()
                        root.exitClicked()
                    }
                }
                
                Button {
                    text: "取消"
                    
                    background: Rectangle {
                        color: "#666666"
                        radius: 5
                    }
                    
                    onClicked: {
                        exitConfirmDialog.close()
                    }
                }
            }
        }
    }
}