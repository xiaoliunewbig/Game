// ============================================================================
// 《幻境传说》游戏框架 - 玩家信息组件
// ============================================================================
// 文件名: PlayerInfo.qml
// 说明: 显示玩家基本信息的UI组件，包括头像、等级、生命值、魔法值等
// 作者: 彭承康
// 创建时间: 2025-07-20
// 版本: v1.0.0
//
// 功能描述:
// - 玩家头像和名称显示
// - 等级和经验值进度条
// - 生命值和魔法值条
// - 状态效果图标显示
// - 实时数据更新和动画效果
//
// 使用场景:
// - 游戏主界面的玩家信息面板
// - 战斗界面的状态显示
// - 角色界面的详细信息
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 玩家信息显示组件
 * @details 提供玩家基础信息的可视化显示，支持实时数据更新和动画效果
 * 
 * 主要功能:
 * - 玩家基本信息展示（姓名、等级、职业）
 * - 生命值/魔法值进度条显示
 * - 经验值进度和升级提示
 * - 状态效果图标管理
 * - 数据变化的平滑动画
 * 
 * 使用示例:
 * @code
 * PlayerInfo {
 *     playerName: "勇者"
 *     playerLevel: 15
 *     currentHealth: 85
 *     maxHealth: 100
 *     onLevelUp: {
 *         // 处理升级逻辑
 *     }
 * }
 * @endcode
 */
Rectangle {
    id: playerInfo
    
    // ------------------------------------------------------------------------
    // 组件尺寸和外观
    // ------------------------------------------------------------------------
    
    width: 280
    height: 120
    color: "#3498db"
    radius: 12
    border.color: "#2980b9"
    border.width: 2
    
    // 渐变背景效果
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#3498db" }
        GradientStop { position: 1.0; color: "#2980b9" }
    }
    
    // 阴影效果
    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 3
        verticalOffset: 3
        radius: 8.0
        samples: 17
        color: "#80000000"
    }
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 玩家姓名
    property string playerName: "未知冒险者"
    
    /// 玩家等级
    property int playerLevel: 1
    
    /// 玩家职业
    property string playerClass: "战士"
    
    /// 当前生命值
    property int currentHealth: 100
    
    /// 最大生命值
    property int maxHealth: 100
    
    /// 当前魔法值
    property int currentMana: 50
    
    /// 最大魔法值
    property int maxMana: 50
    
    /// 当前经验值
    property int currentExp: 0
    
    /// 升级所需经验值
    property int expToNextLevel: 1000
    
    /// 玩家头像路径
    property string avatarSource: "qrc:/resources/images/player_avatar.png"
    
    /// 是否显示详细信息
    property bool showDetails: true
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 玩家升级信号
    signal levelUp(int newLevel)
    
    /// 生命值变化信号
    /// @param oldValue 旧值
    /// @param newValue 新值
    signal healthChanged(int oldValue, int newValue)
    
    /// 魔法值变化信号
    /// @param oldValue 旧值
    /// @param newValue 新值
    signal manaChanged(int oldValue, int newValue)
    
    /// 组件被点击信号
    signal clicked()
    
    // ------------------------------------------------------------------------
    // 主要布局结构
    // ------------------------------------------------------------------------
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 15
        
        // ------------------------------------------------------------------------
        // 左侧头像区域
        // ------------------------------------------------------------------------
        
        Rectangle {
            id: avatarContainer
            Layout.preferredWidth: 80
            Layout.preferredHeight: 80
            Layout.alignment: Qt.AlignVCenter
            
            color: "#ecf0f1"
            radius: 40
            border.color: "#bdc3c7"
            border.width: 3
            
            // 头像图片
            Image {
                id: avatarImage
                anchors.centerIn: parent
                width: 70
                height: 70
                source: playerInfo.avatarSource
                fillMode: Image.PreserveAspectCrop
                
                // 圆形裁剪
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        width: avatarImage.width
                        height: avatarImage.height
                        radius: width / 2
                    }
                }
                
                // 加载失败时显示默认图标
                onStatusChanged: {
                    if (status === Image.Error) {
                        source = ""
                    }
                }
            }
            
            // 默认头像文本（当图片加载失败时显示）
            Text {
                anchors.centerIn: parent
                text: "👤"
                font.pixelSize: 40
                visible: avatarImage.source === ""
                color: "#7f8c8d"
            }
            
            // 等级徽章
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.bottomMargin: -5
                anchors.rightMargin: -5
                
                width: 30
                height: 30
                radius: 15
                color: "#f39c12"
                border.color: "#e67e22"
                border.width: 2
                
                Text {
                    anchors.centerIn: parent
                    text: playerInfo.playerLevel
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 右侧信息区域
        // ------------------------------------------------------------------------
        
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            
            // 玩家姓名和职业
            Column {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: playerInfo.playerName
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                    width: parent.width
                }
                
                Text {
                    text: playerInfo.playerClass + " • Lv." + playerInfo.playerLevel
                    color: "#ecf0f1"
                    font.pixelSize: 12
                    opacity: 0.9
                }
            }
            
            // 生命值条
            ProgressBarWithLabel {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                
                label: "HP"
                currentValue: playerInfo.currentHealth
                maxValue: playerInfo.maxHealth
                barColor: playerInfo.currentHealth > playerInfo.maxHealth * 0.3 ? "#e74c3c" : "#c0392b"
                backgroundColor: "#34495e"
                
                // 生命值变化动画
                Behavior on currentValue {
                    NumberAnimation { 
                        duration: 500
                        easing.type: Easing.OutCubic
                    }
                }
            }
            
            // 魔法值条
            ProgressBarWithLabel {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                
                label: "MP"
                currentValue: playerInfo.currentMana
                maxValue: playerInfo.maxMana
                barColor: "#3498db"
                backgroundColor: "#34495e"
                
                // 魔法值变化动画
                Behavior on currentValue {
                    NumberAnimation { 
                        duration: 500
                        easing.type: Easing.OutCubic
                    }
                }
            }
            
            // 经验值条（可选显示）
            ProgressBarWithLabel {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
                visible: playerInfo.showDetails
                
                label: "EXP"
                currentValue: playerInfo.currentExp
                maxValue: playerInfo.expToNextLevel
                barColor: "#f39c12"
                backgroundColor: "#34495e"
                fontSize: 10
                
                // 经验值变化动画
                Behavior on currentValue {
                    NumberAnimation { 
                        duration: 800
                        easing.type: Easing.OutCubic
                        
                        onFinished: {
                            // 检查是否升级
                            if (playerInfo.currentExp >= playerInfo.expToNextLevel) {
                                checkLevelUp()
                            }
                        }
                    }
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 鼠标交互
    // ------------------------------------------------------------------------
    
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        
        onEntered: {
            playerInfo.scale = 1.02
            playerInfo.color = Qt.lighter(playerInfo.color, 1.1)
        }
        
        onExited: {
            playerInfo.scale = 1.0
            playerInfo.color = "#3498db"
        }
        
        onClicked: {
            console.log("玩家信息面板被点击")
            playerInfo.clicked()
        }
    }
    
    // 缩放动画
    Behavior on scale {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }
    
    // 颜色变化动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    // ------------------------------------------------------------------------
    // 自定义进度条组件
    // ------------------------------------------------------------------------
    
    component ProgressBarWithLabel: Rectangle {
        property string label: ""
        property int currentValue: 0
        property int maxValue: 100
        property color barColor: "#3498db"
        property color backgroundColor: "#34495e"
        property int fontSize: 11
        
        color: backgroundColor
        radius: height / 2
        border.color: Qt.darker(backgroundColor, 1.2)
        border.width: 1
        
        // 进度条填充
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * (currentValue / Math.max(maxValue, 1))
            color: barColor
            radius: parent.radius
            
            // 进度条光泽效果
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.lighter(parent.color, 1.3) }
                    GradientStop { position: 0.5; color: parent.color }
                    GradientStop { position: 1.0; color: Qt.darker(parent.color, 1.2) }
                }
                radius: parent.radius
            }
        }
        
        // 标签和数值文本
        Text {
            anchors.centerIn: parent
            text: label + ": " + currentValue + "/" + maxValue
            color: "white"
            font.pixelSize: fontSize
            font.bold: true
            
            // 文本阴影效果
            style: Text.Outline
            styleColor: "#2c3e50"
        }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化和私有函数
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("PlayerInfo 组件初始化完成")
        console.log("玩家:", playerName, "等级:", playerLevel, "职业:", playerClass)
    }
    
    // 监听生命值变化
    onCurrentHealthChanged: {
        var oldValue = currentHealth // 这里应该保存旧值
        healthChanged(oldValue, currentHealth)
        
        if (currentHealth <= 0) {
            console.log("玩家生命值归零！")
            // 可以添加死亡动画或效果
        }
    }
    
    // 监听魔法值变化
    onCurrentManaChanged: {
        var oldValue = currentMana // 这里应该保存旧值
        manaChanged(oldValue, currentMana)
    }
    
    /**
     * @brief 检查是否升级
     * @details 当经验值达到升级要求时触发升级逻辑
     */
    function checkLevelUp() {
        if (currentExp >= expToNextLevel) {
            var newLevel = playerLevel + 1
            playerLevel = newLevel
            currentExp -= expToNextLevel
            expToNextLevel = Math.floor(expToNextLevel * 1.2) // 下一级所需经验增加20%
            
            console.log("玩家升级到", newLevel, "级！")
            levelUp(newLevel)
            
            // 升级时恢复生命值和魔法值
            currentHealth = maxHealth
            currentMana = maxMana
        }
    }
    
    /**
     * @brief 更新玩家数据
     * @param data 包含玩家信息的对象
     */
    function updatePlayerData(data) {
        if (data.name !== undefined) playerName = data.name
        if (data.level !== undefined) playerLevel = data.level
        if (data.class !== undefined) playerClass = data.class
        if (data.health !== undefined) currentHealth = data.health
        if (data.maxHealth !== undefined) maxHealth = data.maxHealth
        if (data.mana !== undefined) currentMana = data.mana
        if (data.maxMana !== undefined) maxMana = data.maxMana
        if (data.exp !== undefined) currentExp = data.exp
        if (data.expToNext !== undefined) expToNextLevel = data.expToNext
        
        console.log("玩家数据已更新")
    }
    
    /**
     * @brief 播放伤害动画
     * @param damage 伤害数值
     */
    function playDamageAnimation(damage) {
        // 创建伤害数字动画
        var damageText = Qt.createQmlObject('
            import QtQuick 2.15
            Text {
                text: "-" + damage
                color: "#e74c3c"
