/*
 * 文件名: SkillSlot.qml
 * 说明: 技能槽位组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * @brief 技能槽位组件
 * 
 * 显示单个技能槽位，包括：
 * - 技能图标
 * - 冷却时间显示
 * - 键盘绑定提示
 * - 可用状态指示
 */
Rectangle {
    id: skillSlot
    
    // 自定义属性
    property int slotIndex: 0
    property int skillId: 0
    property string keyBinding: ""
    property bool isAvailable: skillId > 0
    property bool isOnCooldown: false
    property int cooldownTime: 0
    property int maxCooldownTime: 5000
    
    // 外观属性
    color: isAvailable ? "#4a4a4a" : "#2a2a2a"
    border.color: isAvailable ? "#666666" : "#444444"
    border.width: 2
    radius: 8
    
    // 状态变化动画
    Behavior on color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    Behavior on border.color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    // 技能图标
    Image {
        id: skillIcon
        anchors.centerIn: parent
        width: parent.width - 12
        height: parent.height - 12
        source: getSkillIcon()
        visible: isAvailable
        opacity: isOnCooldown ? 0.5 : 1.0
        
        // 透明度动画
        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutQuad
            }
        }
    }
    
    // 空槽位提示
    Text {
        anchors.centerIn: parent
        text: "+"
        font.pixelSize: 24
        color: "#666666"
        visible: !isAvailable
    }
    
    // 冷却遮罩
    Rectangle {
        id: cooldownMask
        anchors.fill: skillIcon
        color: "#80000000"
        radius: 4
        visible: isOnCooldown
        
        // 冷却进度指示器
        Rectangle {
            id: cooldownProgress
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height * (1 - cooldownTime / maxCooldownTime)
            color: "#FF5722"
            radius: 4
            opacity: 0.7
        }
    }
    
    // 冷却时间文字
    Text {
        anchors.centerIn: parent
        text: Math.ceil(cooldownTime / 1000).toString()
        font.pixelSize: 16
        font.bold: true
        color: "#FFFFFF"
        visible: isOnCooldown && cooldownTime > 0
        
        // 文字发光效果
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 0
            radius: 8
            samples: 17
            color: "#000000"
        }
    }
    
    // 键盘绑定提示
    Rectangle {
        id: keyBindingHint
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 2
        width: 16
        height: 16
        color: "#555555"
        border.color: "#777777"
        border.width: 1
        radius: 2
        visible: keyBinding !== ""
        
        Text {
            anchors.centerIn: parent
            text: keyBinding
            font.pixelSize: 10
            font.bold: true
            color: "#FFFFFF"
        }
    }
    
    // 可用状态指示器
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 2
        width: 8
        height: 8
        radius: 4
        color: isAvailable && !isOnCooldown ? "#4CAF50" : "#F44336"
        visible: isAvailable
        
        // 闪烁动画（可用时）
        SequentialAnimation on opacity {
            running: isAvailable && !isOnCooldown
            loops: Animation.Infinite
            NumberAnimation { to: 0.3; duration: 1000 }
            NumberAnimation { to: 1.0; duration: 1000 }
        }
    }
    
    // 鼠标交互
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: isAvailable ? Qt.PointingHandCursor : Qt.ArrowCursor
        
        onClicked: {
            if (isAvailable && !isOnCooldown) {
                useSkill()
            }
        }
        
        onEntered: {
            if (isAvailable) {
                hoverAnimation.to = 1.1
                hoverAnimation.start()
            }
        }
        
        onExited: {
            hoverAnimation.to = 1.0
            hoverAnimation.start()
        }
    }
    
    // 悬停动画
    NumberAnimation {
        id: hoverAnimation
        target: skillSlot
        property: "scale"
        duration: 150
        easing.type: Easing.OutQuad
    }
    
    // 冷却计时器
    Timer {
        id: cooldownTimer
        interval: 100
        running: isOnCooldown
        repeat: true
        
        onTriggered: {
            if (cooldownTime > 0) {
                cooldownTime -= interval
            } else {
                isOnCooldown = false
                cooldownTime = 0
            }
        }
    }
    
    // 工具提示
    ToolTip {
        id: skillToolTip
        text: getSkillTooltip()
        visible: parent.hovered && isAvailable
        delay: 500
        timeout: 5000
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 1
            radius: 4
        }
        
        contentItem: Text {
            text: skillToolTip.text
            font.pixelSize: 12
            color: "#FFFFFF"
            wrapMode: Text.WordWrap
        }
    }
    
    // 获取技能图标
    function getSkillIcon() {
        if (!isAvailable) return ""
        
        switch (skillId) {
            case 1: return "qrc:/resources/icons/skill_attack.png"
            case 2: return "qrc:/resources/icons/skill_heal.png"
            case 3: return "qrc:/resources/icons/skill_fireball.png"
            default: return "qrc:/resources/icons/skill_default.png"
        }
    }
    
    // 获取技能提示信息
    function getSkillTooltip() {
        if (!isAvailable) return ""
        
        switch (skillId) {
            case 1: return "强力攻击\n造成150%物理伤害\n冷却时间: 3秒"
            case 2: return "治疗术\n恢复50点生命值\n冷却时间: 5秒"
            case 3: return "火球术\n造成火焰魔法伤害\n冷却时间: 4秒"
            default: return "未知技能"
        }
    }
    
    // 使用技能
    function useSkill() {
        if (!isAvailable || isOnCooldown) return
        
        // 开始冷却
        isOnCooldown = true
        cooldownTime = maxCooldownTime
        
        // 播放使用动画
        useAnimation.start()
        
        // 这里应该调用实际的技能使用逻辑
        console.log("使用技能:", skillId)
    }
    
    // 技能使用动画
    SequentialAnimation {
        id: useAnimation
        
        NumberAnimation {
            target: skillSlot
            property: "scale"
            to: 1.2
            duration: 100
            easing.type: Easing.OutQuad
        }
        
        NumberAnimation {
            target: skillSlot
            property: "scale"
            to: 1.0
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
}
