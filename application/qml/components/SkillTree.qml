// ============================================================================
// 《幻境传说》游戏框架 - 技能树组件
// ============================================================================
// 文件名: SkillTree.qml
// 说明: 玩家技能树显示和管理组件，支持技能学习、升级、预览等功能
// 作者: 彭承康
// 创建时间: 2026-02-18
// 版本: v1.0.0
//
// 功能描述:
// - 技能树节点显示和连接线绘制
// - 技能学习条件检查和学习功能
// - 技能详情预览和描述显示
// - 技能点分配和重置功能
// - 不同职业技能树切换
//
// 使用场景:
// - 角色界面的技能管理
// - 升级时的技能点分配
// - 技能预览和规划
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 技能树显示组件
 * @details 提供完整的技能树界面，包括技能节点、连接关系、学习管理等功能
 * 
 * 主要功能:
 * - 技能树结构化显示
 * - 技能学习状态管理
 * - 技能点消耗和分配
 * - 技能详情查看
 * - 技能树重置功能
 * 
 * 使用示例:
 * @code
 * SkillTree {
 *     playerClass: "warrior"
 *     availableSkillPoints: 5
 *     onSkillLearned: function(skillId) {
 *         // 处理技能学习逻辑
 *     }
 * }
 * @endcode
 */
Rectangle {
    id: skillTree
    
    // ------------------------------------------------------------------------
    // 组件尺寸和外观
    // ------------------------------------------------------------------------
    
    width: 600
    height: 400
    color: "#2c3e50"
    radius: 12
    border.color: "#34495e"
    border.width: 2
    
    // 背景渐变效果
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#2c3e50" }
        GradientStop { position: 1.0; color: "#34495e" }
    }
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 玩家职业类型
    property string playerClass: "warrior"
    
    /// 可用技能点数
    property int availableSkillPoints: 0
    
    /// 已使用技能点数
    property int usedSkillPoints: 0
    
    /// 当前选中的技能ID
    property int selectedSkillId: -1
    
    /// 是否显示技能详情面板
    property bool showSkillDetails: false
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 技能被学习信号
    /// @param skillId 技能ID
    /// @param skillLevel 技能等级
    signal skillLearned(int skillId, int skillLevel)
    
    /// 技能被选中信号
    /// @param skillId 技能ID
    signal skillSelected(int skillId)
    
    /// 技能树重置信号
    signal skillTreeReset()
    
    /// 技能点不足信号
    signal insufficientSkillPoints()
    
    // ------------------------------------------------------------------------
    // 主界面布局
    // ------------------------------------------------------------------------
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15
        
        // ------------------------------------------------------------------------
        // 顶部信息栏
        // ------------------------------------------------------------------------
        
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#34495e"
            radius: 8
            border.color: "#7f8c8d"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20
                
                // 职业图标和名称
                Row {
                    spacing: 10
                    
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: getClassColor(skillTree.playerClass)
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            anchors.centerIn: parent
                            text: getClassIcon(skillTree.playerClass)
                            font.pixelSize: 20
                            color: "white"
                        }
                    }
                    
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        
                        Text {
                            text: getClassName(skillTree.playerClass)
                            color: "#ecf0f1"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        Text {
                            text: "技能树"
                            color: "#bdc3c7"
                            font.pixelSize: 12
                        }
                    }
                }
                
                Item { Layout.fillWidth: true } // 弹性空间
                
                // 技能点信息
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    
                    Text {
                        text: "可用技能点: " + skillTree.availableSkillPoints
                        color: skillTree.availableSkillPoints > 0 ? "#2ecc71" : "#95a5a6"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                    }
                    
                    Text {
                        text: "已使用: " + skillTree.usedSkillPoints
                        color: "#bdc3c7"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignRight
                    }
                }
                
                // 重置按钮
                Button {
                    text: "重置"
                    Layout.preferredWidth: 60
                    Layout.preferredHeight: 35
                    enabled: skillTree.usedSkillPoints > 0
                    
                    background: Rectangle {
                        color: parent.enabled ? 
                               (parent.pressed ? "#c0392b" : "#e74c3c") : "#7f8c8d"
                        radius: 6
                        border.color: parent.enabled ? "#a93226" : "#95a5a6"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: parent.enabled ? "white" : "#bdc3c7"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        resetSkillTree()
                    }
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 技能树主体区域
        // ------------------------------------------------------------------------
        
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#34495e"
            radius: 8
            border.color: "#7f8c8d"
            border.width: 1
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                
                // 技能树画布
                Item {
                    id: skillTreeCanvas
                    width: Math.max(scrollView.width, 800)
                    height: Math.max(scrollView.height, 600)
                    
                    // 技能节点数据模型
                    property var skillNodes: getSkillNodesForClass(skillTree.playerClass)
                    
                    // 绘制连接线
                    Canvas {
                        id: connectionCanvas
                        anchors.fill: parent
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            
                            // 绘制技能节点之间的连接线
                            drawSkillConnections(ctx)
                        }
                        
                        Component.onCompleted: requestPaint()
                    }
                    
                    // 技能节点重复器
                    Repeater {
                        model: skillTreeCanvas.skillNodes
                        
                        delegate: SkillNode {
                            x: modelData.x
                            y: modelData.y
                            skillId: modelData.id
                            skillName: modelData.name
                            skillIcon: modelData.icon
                            skillLevel: modelData.currentLevel
                            maxLevel: modelData.maxLevel
                            isLearned: modelData.learned
                            isAvailable: modelData.available
                            requiredLevel: modelData.requiredLevel
                            skillPointCost: modelData.cost
                            
                            onClicked: {
                                selectSkill(skillId)
                            }
                            
                            onLearnRequested: {
                                learnSkill(skillId)
                            }
                        }
                    }
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 技能详情面板（悬浮显示）
    // ------------------------------------------------------------------------
    
    Rectangle {
        id: skillDetailsPanel
        width: 300
        height: 200
        color: "#2c3e50"
        radius: 8
        border.color: "#3498db"
        border.width: 2
        visible: skillTree.showSkillDetails && skillTree.selectedSkillId >= 0
        z: 100
        
        // 跟随鼠标位置
        x: Math.min(parent.width - width - 10, Math.max(10, mouseX + 20))
        y: Math.min(parent.height - height - 10, Math.max(10, mouseY - height/2))
        
        property int mouseX: 0
        property int mouseY: 0
        
        Column {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10
            
            // 技能名称
            Text {
                text: getSkillName(skillTree.selectedSkillId)
                color: "#ecf0f1"
                font.pixelSize: 16
                font.bold: true
            }
            
            // 技能描述
            Text {
                text: getSkillDescription(skillTree.selectedSkillId)
                color: "#bdc3c7"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                width: parent.width
            }
            
            // 技能等级信息
            Text {
                text: "等级: " + getSkillCurrentLevel(skillTree.selectedSkillId) + 
                      "/" + getSkillMaxLevel(skillTree.selectedSkillId)
                color: "#f39c12"
                font.pixelSize: 12
            }
            
            // 学习条件
            Text {
                text: "需要等级: " + getSkillRequiredLevel(skillTree.selectedSkillId)
                color: "#95a5a6"
                font.pixelSize: 11
            }
            
            // 技能点消耗
            Text {
                text: "消耗技能点: " + getSkillCost(skillTree.selectedSkillId)
                color: "#e74c3c"
                font.pixelSize: 11
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 自定义技能节点组件
    // ------------------------------------------------------------------------
    
    component SkillNode: Rectangle {
        property int skillId: 0
        property string skillName: ""
        property string skillIcon: "⚔️"
        property int skillLevel: 0
        property int maxLevel: 5
        property bool isLearned: false
        property bool isAvailable: true
        property int requiredLevel: 1
        property int skillPointCost: 1
        
        signal clicked(int skillId)
        signal learnRequested(int skillId)
        
        width: 60
        height: 60
        radius: 30
        
        // 根据状态设置颜色
        color: {
            if (isLearned) return "#2ecc71"
            if (isAvailable) return "#3498db"
            return "#7f8c8d"
        }
        
        border.color: skillTree.selectedSkillId === skillId ? "#f39c12" : Qt.darker(color, 1.3)
        border.width: skillTree.selectedSkillId === skillId ? 3 : 2
        
        // 技能图标
        Text {
            anchors.centerIn: parent
            text: skillIcon
            font.pixelSize: 24
            color: "white"
        }
        
        // 技能等级显示
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.bottomMargin: -5
            anchors.rightMargin: -5
            width: 20
            height: 20
            radius: 10
            color: "#e67e22"
            border.color: "#d35400"
            border.width: 1
            visible: isLearned && skillLevel > 0
            
            Text {
                anchors.centerIn: parent
                text: skillLevel
                color: "white"
                font.pixelSize: 10
                font.bold: true
            }
        }
        
        // 鼠标交互
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            
            onEntered: {
                parent.scale = 1.1
                skillTree.selectedSkillId = skillId
                skillTree.showSkillDetails = true
                skillDetailsPanel.mouseX = mouseX + parent.x
                skillDetailsPanel.mouseY = mouseY + parent.y
            }
            
            onExited: {
                parent.scale = 1.0
                skillTree.showSkillDetails = false
            }
            
            onClicked: {
                parent.clicked(skillId)
            }
            
            onDoubleClicked: {
                if (isAvailable && !isLearned && skillTree.availableSkillPoints >= skillPointCost) {
                    parent.learnRequested(skillId)
                }
            }
            
            onPositionChanged: {
                if (skillTree.showSkillDetails) {
                    skillDetailsPanel.mouseX = mouseX + parent.x
                    skillDetailsPanel.mouseY = mouseY + parent.y
                }
            }
        }
        
        // 缩放动画
        Behavior on scale {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }
        
        // 颜色变化动画
        Behavior on color {
            ColorAnimation { duration: 300 }
        }
        
        // 边框变化动画
        Behavior on border.color {
            ColorAnimation { duration: 200 }
        }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化和私有函数
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("SkillTree 组件初始化完成")
        console.log("职业:", playerClass, "可用技能点:", availableSkillPoints)
    }
    
    /**
     * @brief 获取职业对应的颜色
     * @param className 职业名称
     * @return 颜色值
     */
    function getClassColor(className) {
        switch(className) {
            case "warrior": return "#e74c3c"
            case "mage": return "#9b59b6"
            case "archer": return "#2ecc71"
            default: return "#95a5a6"
        }
    }
    
    /**
     * @brief 获取职业对应的图标
     * @param className 职业名称
     * @return 图标字符
     */
    function getClassIcon(className) {
        switch(className) {
            case "warrior": return "⚔️"
            case "mage": return "🔮"
            case "archer": return "🏹"
            default: return "❓"
        }
    }
    
    /**
     * @brief 获取职业显示名称
     * @param className 职业名称
     * @return 显示名称
     */
    function getClassName(className) {
        switch(className) {
            case "warrior": return "战士"
            case "mage": return "法师"
            case "archer": return "弓箭手"
            default: return "未知职业"
        }
    }
    
    /**
     * @brief 获取指定职业的技能节点数据
     * @param className 职业名称
     * @return 技能节点数组
     */
    function getSkillNodesForClass(className) {
        // 这里返回模拟数据，实际应该从游戏数据中获取
        var nodes = []
        
        if (className === "warrior") {
            nodes = [
                {id: 1, name: "重击", icon: "⚔️", x: 100, y: 50, currentLevel: 0, maxLevel: 5, learned: false, available: true, requiredLevel: 1, cost: 1},
                {id: 2, name: "防御", icon: "🛡️", x: 200, y: 50, currentLevel: 0, maxLevel: 3, learned: false, available: true, requiredLevel: 1, cost: 1},
                {id: 3, name: "狂暴", icon: "💢", x: 150, y: 150, currentLevel: 0, maxLevel: 3, learned: false, available: false, requiredLevel: 5, cost: 2},
                {id: 4, name: "旋风斩", icon: "🌪️", x: 100, y: 250, currentLevel: 0, maxLevel: 1, learned: false, available: false, requiredLevel: 10, cost: 3}
            ]
        } else if (className === "mage") {
            nodes = [
                {id: 11, name: "火球术", icon: "🔥", x: 100, y: 50, currentLevel: 0, maxLevel: 5, learned: false, available: true, requiredLevel: 1, cost: 1},
                {id: 12, name: "冰箭", icon: "❄️", x: 200, y: 50, currentLevel: 0, maxLevel: 5, learned: false, available: true, requiredLevel: 1, cost: 1},
                {id: 13, name: "闪电", icon: "⚡", x: 300, y: 50, currentLevel: 0, maxLevel: 5, learned: false, available: true, requiredLevel: 1, cost: 1},
                {id: 14, name: "魔法护盾", icon: "🔮", x: 150, y: 150, currentLevel: 0, maxLevel: 3, learned: false, available: false, requiredLevel: 5, cost: 2}
            ]
        }
        
        return nodes
    }
    
    /**
     * @brief 绘制技能连接线
     * @param ctx Canvas绘图上下文
     */
    function drawSkillConnections(ctx) {
        ctx.strokeStyle = "#7f8c8d"
        ctx.lineWidth = 2
        
        // 根据职业绘制不同的连接线
        if (playerClass === "warrior") {
            // 重击 -> 狂暴
            drawConnection(ctx, 130, 80, 150, 150)
            // 防御 -> 狂暴
            drawConnection(ctx, 200, 80, 180, 150)
            // 狂暴 -> 旋风斩
            drawConnection(ctx, 150, 180, 130, 250)
        }
    }
    
    /**
     * @brief 绘制两点之间的连接线
     * @param ctx 绘图上下文
     * @param x1 起点X
     * @param y1 起点Y
     * @param x2 终点X
     * @param y2 终点Y
     */
    function drawConnection(ctx, x1, y1, x2, y2) {
        ctx.beginPath()
        ctx.moveTo(x1, y1)
        ctx.lineTo(x2, y2)
        ctx.stroke()
    }
    
    /**
     * @brief 选择技能
     * @param skillId 技能ID
     */
    function selectSkill(skillId) {
        selectedSkillId = skillId
        skillSelected(skillId)
        console.log("选择技能:", skillId)
    }
    
    /**
     * @brief 学习技能
     * @param skillId 技能ID
     */
    function learnSkill(skillId) {
        if (availableSkillPoints <= 0) {
            insufficientSkillPoints()
            console.log("技能点不足")
            return
        }
        
        // 查找技能节点并更新状态
        var nodes = skillTreeCanvas.skillNodes
        for (var i = 0; i < nodes.length; i++) {
            if (nodes[i].id === skillId && nodes[i].available && !nodes[i].learned) {
                nodes[i].learned = true
                nodes[i].currentLevel = 1
                availableSkillPoints -= nodes[i].cost
                usedSkillPoints += nodes[i].cost
                
                skillLearned(skillId, 1)
                console.log("学习技能:", skillId)
                
                // 更新其他技能的可用状态
                updateSkillAvailability()
                break
            }
        }
    }
    
    /**
     * @brief 重置技能树
     */
    function resetSkillTree() {
        console.log("重置技能树")
        
        // 重置所有技能状态
        var nodes = skillTreeCanvas.skillNodes
        for (var i = 0; i < nodes.length; i++) {
            nodes[i].learned = false
            nodes[i].currentLevel = 0
            nodes[i].available = (nodes[i].requiredLevel <= 1)
        }
        
        // 重置技能点
        availableSkillPoints += usedSkillPoints
        usedSkillPoints = 0
        selectedSkillId = -1
        
        skillTreeReset()
    }
    
    /**
     * @brief 更新技能可用状态
     */
    function updateSkillAvailability() {
        // 这里应该根据已学习的技能更新其他技能的可用状态
        console.log("更新技能可用状态")
    }
    
    // 以下是获取技能信息的辅助函数
    function getSkillName(skillId) { return "技能名称" }
    function getSkillDescription(skillId) { return "技能描述..." }
    function getSkillCurrentLevel(skillId) { return 0 }
    function getSkillMaxLevel(skillId) { return 5 }
    function getSkillRequiredLevel(skillId) { return 1 }
    function getSkillCost(skillId) { return 1 }
}
