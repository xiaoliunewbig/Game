// ============================================================================
// 《幻境传说》游戏框架 - 物品卡片组件
// ============================================================================
// 文件名: ItemCard.qml
// 说明: 物品信息卡片，显示物品图标、名称、品质和数量
// 作者: 彭承康
// 创建时间: 2026-02-18
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: itemCard

    width: 100
    height: 130
    radius: 8
    color: mouseArea.containsMouse ? "#3d566e" : (hasItem ? "#34495e" : "#7f8c8d")
    border.color: hasItem ? getQualityColor(itemQuality) : "#95a5a6"
    border.width: selected ? 3 : (hasItem ? 2 : 1)

    // 公共属性
    property int itemId: 0
    property string itemName: ""
    property string itemIcon: ""
    property int itemQuantity: 0
    property int itemQuality: 0
    property string itemType: ""
    property string itemDescription: ""
    property bool selected: false

    readonly property bool hasItem: itemId > 0

    // 信号
    signal itemClicked(int itemId)
    signal itemDoubleClicked(int itemId)

    // 主内容
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4

        // 图标区域
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            radius: 6
            color: hasItem ? "#2c3e50" : "#6c7a89"
            border.color: hasItem ? getQualityColor(itemQuality) : "transparent"
            border.width: 1

            // 物品图片
            Image {
                id: iconImage
                anchors.centerIn: parent
                width: 44
                height: 44
                source: itemCard.hasItem ? itemCard.itemIcon : ""
                fillMode: Image.PreserveAspectFit
                visible: itemCard.hasItem && status === Image.Ready

                onStatusChanged: {
                    if (status === Image.Error && itemCard.hasItem) {
                        source = ""
                    }
                }
            }

            // 默认图标（无图片时）
            Text {
                anchors.centerIn: parent
                text: hasItem ? getDefaultIcon(itemCard.itemType) : "+"
                font.pixelSize: hasItem ? 28 : 20
                color: hasItem ? "#bdc3c7" : "#95a5a6"
                visible: !iconImage.visible
                opacity: hasItem ? 1.0 : 0.5
            }
        }

        // 物品名称
        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 16
            text: hasItem ? itemCard.itemName : ""
            color: hasItem ? getQualityColor(itemQuality) : "#95a5a6"
            font.pixelSize: 11
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        // 类型标签
        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 14
            text: hasItem ? getTypeName(itemCard.itemType) : ""
            color: "#95a5a6"
            font.pixelSize: 9
            horizontalAlignment: Text.AlignHCenter
            visible: hasItem
        }
    }

    // 数量徽章
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 4
        anchors.rightMargin: 4
        width: qtyText.width + 8
        height: qtyText.height + 4
        radius: 3
        color: "#2c3e50"
        border.color: "#34495e"
        border.width: 1
        visible: hasItem && itemCard.itemQuantity > 1

        Text {
            id: qtyText
            anchors.centerIn: parent
            text: itemCard.itemQuantity > 999 ? "999+" : itemCard.itemQuantity.toString()
            color: "white"
            font.pixelSize: 10
            font.bold: true
        }
    }

    // 选中指示器
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: "#f39c12"
        border.width: 3
        visible: itemCard.selected
    }

    // 品质光效
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: getQualityColor(itemCard.itemQuality)
        border.width: 1
        opacity: 0.3
        visible: hasItem && itemCard.itemQuality >= 3

        SequentialAnimation on opacity {
            running: visible
            loops: Animation.Infinite
            NumberAnimation { to: 0.1; duration: 1500 }
            NumberAnimation { to: 0.5; duration: 1500 }
        }
    }

    // 交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (hasItem) itemCard.itemClicked(itemCard.itemId)
        }
        onDoubleClicked: {
            if (hasItem) itemCard.itemDoubleClicked(itemCard.itemId)
        }
        onEntered: itemCard.scale = 1.05
        onExited: itemCard.scale = 1.0
    }

    // 动画
    Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }

    // 工具函数
    function getQualityColor(quality) {
        switch(quality) {
            case 0: return "#95a5a6"
            case 1: return "#ffffff"
            case 2: return "#2ecc71"
            case 3: return "#3498db"
            case 4: return "#9b59b6"
            case 5: return "#f39c12"
            default: return "#95a5a6"
        }
    }

    function getDefaultIcon(type) {
        switch(type) {
            case "weapon": return "\u2694\uFE0F"
            case "armor": return "\uD83D\uDEE1\uFE0F"
            case "consumable": return "\uD83E\uDDEA"
            case "material": return "\uD83D\uDD27"
            case "quest": return "\uD83D\uDCDC"
            case "misc": return "\uD83D\uDCE6"
            default: return "\u2753"
        }
    }

    function getTypeName(type) {
        switch(type) {
            case "weapon": return "武器"
            case "armor": return "防具"
            case "consumable": return "消耗品"
            case "material": return "材料"
            case "quest": return "任务"
            case "misc": return "杂物"
            default: return ""
        }
    }

    function setItemData(data) {
        if (data && data.id > 0) {
            itemId = data.id
            itemIcon = data.icon || ""
            itemName = data.name || ""
            itemQuantity = data.quantity || 1
            itemQuality = data.quality || 0
            itemType = data.type || ""
            itemDescription = data.description || ""
        } else {
            clearItem()
        }
    }

    function clearItem() {
        itemId = 0; itemIcon = ""; itemName = ""
        itemQuantity = 0; itemQuality = 0
        itemType = ""; itemDescription = ""
        selected = false
    }
}
