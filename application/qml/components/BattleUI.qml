// ============================================================================
// 《幻境传说》游戏框架 - 战斗操作界面组件
// ============================================================================
// 文件名: BattleUI.qml
// 说明: 战斗操作面板，包含敌人信息、技能栏、动作按钮和战斗日志
// 作者: 彭承康
// 创建时间: 2026-02-18
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: battleUI

    color: "transparent"

    // 公共属性
    property string enemyName: ""
    property int enemyLevel: 1
    property int enemyHealth: 100
    property int enemyMaxHealth: 100
    property string enemyElement: ""
    property var enemyStatuses: []

    property int playerHealth: 100
    property int playerMaxHealth: 100
    property int playerMana: 50
    property int playerMaxMana: 50

    property bool battleActive: false
    property int currentTurn: 1

    // 技能数据模型
    property var skillSlots: [
        { id: 1, name: "斩击", icon: "\u2694\uFE0F", cooldown: 0, maxCooldown: 0, key: "1" },
        { id: 2, name: "火球", icon: "\uD83D\uDD25", cooldown: 0, maxCooldown: 3000, key: "2" },
        { id: 3, name: "治愈", icon: "\uD83D\uDC9A", cooldown: 0, maxCooldown: 5000, key: "3" },
        { id: 4, name: "护盾", icon: "\uD83D\uDEE1\uFE0F", cooldown: 0, maxCooldown: 8000, key: "4" }
    ]

    // 战斗日志模型
    property alias battleLogModel: logModel

    // 信号
    signal skillSelected(int skillId)
    signal actionSelected(string actionType)
    signal retreatRequested()

    // 主布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // 区域1：敌人信息
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 130
            radius: 8
            color: "#2c3040"
            border.color: "#555"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                // 标题行
                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "\u2620\uFE0F 敌人"
                        color: "#e74c3c"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "回合 " + battleUI.currentTurn
                        color: "#95a5a6"
                        font.pixelSize: 11
                    }
                }

                // 敌人卡片
                EnemyCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enemyName: battleUI.enemyName
                    enemyLevel: battleUI.enemyLevel
                    currentHealth: battleUI.enemyHealth
                    maxHealth: battleUI.enemyMaxHealth
                    elementType: battleUI.enemyElement
                    statusEffects: battleUI.enemyStatuses

                    onEnemyClicked: {
                        console.log("BattleUI: 点击敌人")
                    }
                }
            }
        }

        // 区域2：玩家状态栏（简化）
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 6
            color: "#2c3040"
            border.color: "#555"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 8

                // HP
                RowLayout {
                    spacing: 4
                    Text {
                        text: "\u2764\uFE0F"
                        font.pixelSize: 12
                    }
                    Rectangle {
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 12
                        radius: 3
                        color: "#1a1a2e"
                        border.color: "#444"
                        border.width: 1

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 1
                            width: Math.max(0, (parent.width - 2) * (battleUI.playerMaxHealth > 0 ? battleUI.playerHealth / battleUI.playerMaxHealth : 0))
                            radius: 2
                            color: "#e74c3c"
                            Behavior on width { NumberAnimation { duration: 200 } }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: battleUI.playerHealth + "/" + battleUI.playerMaxHealth
                            color: "white"
                            font.pixelSize: 8
                            font.bold: true
                        }
                    }
                }

                // MP
                RowLayout {
                    spacing: 4
                    Text {
                        text: "\uD83D\uDCA7"
                        font.pixelSize: 12
                    }
                    Rectangle {
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 12
                        radius: 3
                        color: "#1a1a2e"
                        border.color: "#444"
                        border.width: 1

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 1
                            width: Math.max(0, (parent.width - 2) * (battleUI.playerMaxMana > 0 ? battleUI.playerMana / battleUI.playerMaxMana : 0))
                            radius: 2
                            color: "#3498db"
                            Behavior on width { NumberAnimation { duration: 200 } }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: battleUI.playerMana + "/" + battleUI.playerMaxMana
                            color: "white"
                            font.pixelSize: 8
                            font.bold: true
                        }
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

        // 区域3：技能操作栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            radius: 6
            color: "#2c3040"
            border.color: "#555"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 4

                // 技能栏标题
                Text {
                    text: "\u2694\uFE0F 技能"
                    color: "#f39c12"
                    font.pixelSize: 11
                    font.bold: true
                }

                // 技能按钮行
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    // 4个技能槽
                    Repeater {
                        model: battleUI.skillSlots

                        ActionButton {
                            Layout.preferredWidth: 60
                            Layout.preferredHeight: 60
                            actionId: modelData.id
                            actionName: modelData.name
                            actionIcon: modelData.icon
                            cooldownTotal: modelData.maxCooldown
                            shortcutKey: modelData.key
                            enabled: battleUI.battleActive

                            onActionTriggered: function(id) {
                                battleUI.skillSelected(id)
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // 防御按钮
                    ActionButton {
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                        actionId: 100
                        actionName: "防御"
                        actionIcon: "\uD83D\uDEE1\uFE0F"
                        shortcutKey: "D"
                        enabled: battleUI.battleActive

                        onActionTriggered: {
                            battleUI.actionSelected("defend")
                        }
                    }

                    // 逃跑按钮
                    ActionButton {
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                        actionId: 101
                        actionName: "逃跑"
                        actionIcon: "\uD83C\uDFC3"
                        shortcutKey: "R"
                        enabled: battleUI.battleActive

                        onActionTriggered: {
                            battleUI.retreatRequested()
                        }
                    }
                }
            }
        }

        // 区域4：战斗日志
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 80
            radius: 6
            color: "#1a1a2e"
            border.color: "#555"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 4

                // 日志标题
                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "\uD83D\uDCDC 战斗日志"
                        color: "#95a5a6"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    // 清除按钮
                    Text {
                        text: "清除"
                        color: mouseAreaClear.containsMouse ? "#e74c3c" : "#7f8c8d"
                        font.pixelSize: 10

                        MouseArea {
                            id: mouseAreaClear
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: logModel.clear()
                        }
                    }
                }

                // 日志列表
                ListView {
                    id: logListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2

                    model: ListModel {
                        id: logModel
                    }

                    delegate: Text {
                        width: logListView.width
                        text: message
                        color: msgColor
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    // 自动滚动到底部
                    onCountChanged: {
                        Qt.callLater(function() {
                            logListView.positionViewAtEnd()
                        })
                    }
                }
            }
        }
    }

    // 公共函数
    function addBattleLog(text, type) {
        var color = "#bdc3c7"
        switch(type) {
            case "damage":  color = "#e74c3c"; break
            case "heal":    color = "#2ecc71"; break
            case "skill":   color = "#3498db"; break
            case "system":  color = "#f1c40f"; break
            case "buff":    color = "#9b59b6"; break
            case "miss":    color = "#95a5a6"; break
        }
        logModel.append({ "message": text, "msgColor": color })

        // 限制日志条数
        while (logModel.count > 100) {
            logModel.remove(0)
        }
    }

    function clearBattleLog() {
        logModel.clear()
    }

    function startBattle(enemy) {
        battleActive = true
        currentTurn = 1
        if (enemy) {
            enemyName = enemy.name || ""
            enemyLevel = enemy.level || 1
            enemyHealth = enemy.health || 100
            enemyMaxHealth = enemy.maxHealth || 100
            enemyElement = enemy.element || ""
            enemyStatuses = enemy.statuses || []
        }
        addBattleLog("--- 战斗开始！---", "system")
        addBattleLog("遭遇了 " + enemyName + " (Lv." + enemyLevel + ")", "system")
    }

    function endBattle(victory) {
        battleActive = false
        if (victory) {
            addBattleLog("--- 战斗胜利！---", "system")
        } else {
            addBattleLog("--- 战斗结束 ---", "system")
        }
    }

    function nextTurn() {
        currentTurn += 1
        addBattleLog("--- 第 " + currentTurn + " 回合 ---", "system")
    }
}
