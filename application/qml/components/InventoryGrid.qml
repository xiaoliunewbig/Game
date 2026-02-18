// ============================================================================
// 《幻境传说》游戏框架 - 背包网格组件
// ============================================================================
// 文件名: InventoryGrid.qml
// 说明: 背包物品网格布局，使用GridView展示物品槽位
// 作者: 彭承康
// 创建时间: 2026-02-18
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: inventoryGrid

    color: "#2c3e50"
    radius: 8
    border.color: "#555"
    border.width: 1

    // 公共属性
    property int columns: 5
    property int slotSize: 64
    property int slotSpacing: 4
    property int selectedIndex: -1
    property int capacity: 25

    // 物品数据模型
    property alias inventoryModel: gridModel

    // 信号
    signal slotClicked(int index)
    signal slotDoubleClicked(int index)
    signal slotRightClicked(int index)

    // 标题栏
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        radius: 8
        color: "#34495e"

        // 底部直角（只有顶部圆角）
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.radius
            color: parent.color
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            Text {
                text: "\uD83C\uDF92 背包"
                color: "#ecf0f1"
                font.pixelSize: 12
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Text {
                text: getUsedSlots() + "/" + inventoryGrid.capacity
                color: getUsedSlots() >= inventoryGrid.capacity ? "#e74c3c" : "#95a5a6"
                font.pixelSize: 11
            }
        }
    }

    // 网格视图
    GridView {
        id: gridView
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        anchors.topMargin: 6

        cellWidth: inventoryGrid.slotSize + inventoryGrid.slotSpacing
        cellHeight: inventoryGrid.slotSize + inventoryGrid.slotSpacing
        clip: true

        model: ListModel {
            id: gridModel

            Component.onCompleted: {
                // 初始化空槽位
                for (var i = 0; i < inventoryGrid.capacity; i++) {
                    append({
                        "slotItemId": 0,
                        "slotItemName": "",
                        "slotItemIcon": "",
                        "slotItemQuantity": 0,
                        "slotItemQuality": 0,
                        "slotItemType": "",
                        "slotItemDescription": ""
                    })
                }
            }
        }

        delegate: Rectangle {
            id: slotDelegate
            width: inventoryGrid.slotSize
            height: inventoryGrid.slotSize
            radius: 6
            color: {
                if (index === inventoryGrid.selectedIndex) return "#4a6278"
                if (slotMouseArea.containsMouse) return "#3d566e"
                if (slotItemId > 0) return "#34495e"
                return "#2c3e50"
            }
            border.color: {
                if (index === inventoryGrid.selectedIndex) return "#f39c12"
                if (slotItemId > 0) return getQualityColor(slotItemQuality)
                return "#555"
            }
            border.width: index === inventoryGrid.selectedIndex ? 2 : 1

            readonly property bool hasItem: slotItemId > 0

            // 物品图标
            Image {
                id: itemImage
                anchors.centerIn: parent
                width: parent.width * 0.65
                height: parent.height * 0.65
                source: hasItem ? slotItemIcon : ""
                fillMode: Image.PreserveAspectFit
                visible: hasItem && status === Image.Ready

                onStatusChanged: {
                    if (status === Image.Error && hasItem) {
                        source = ""
                    }
                }
            }

            // 默认文字图标（无图片时）
            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -4
                text: hasItem ? getDefaultIcon(slotItemType) : ""
                font.pixelSize: hasItem ? 24 : 16
                color: hasItem ? "#bdc3c7" : "#555"
                visible: !itemImage.visible
            }

            // 空槽位占位符
            Text {
                anchors.centerIn: parent
                text: "+"
                font.pixelSize: 16
                color: "#555"
                visible: !hasItem
                opacity: 0.5
            }

            // 数量徽章
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.bottomMargin: 2
                anchors.rightMargin: 2
                width: qtyText.width + 6
                height: qtyText.height + 2
                radius: 3
                color: "#2c3e50"
                border.color: "#444"
                border.width: 1
                visible: hasItem && slotItemQuantity > 1

                Text {
                    id: qtyText
                    anchors.centerIn: parent
                    text: slotItemQuantity > 999 ? "999+" : slotItemQuantity.toString()
                    color: "white"
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            // 品质指示条（底部彩色细线）
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 4
                height: 2
                radius: 1
                color: getQualityColor(slotItemQuality)
                visible: hasItem && slotItemQuality >= 2
            }

            // 提示
            ToolTip.visible: slotMouseArea.containsMouse && hasItem
            ToolTip.text: slotItemName + (slotItemQuantity > 1 ? " x" + slotItemQuantity : "")
            ToolTip.delay: 500

            // 交互
            MouseArea {
                id: slotMouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: function(mouse) {
                    inventoryGrid.selectedIndex = index
                    if (mouse.button === Qt.RightButton) {
                        inventoryGrid.slotRightClicked(index)
                    } else {
                        inventoryGrid.slotClicked(index)
                    }
                }

                onDoubleClicked: {
                    inventoryGrid.slotDoubleClicked(index)
                }

                onEntered: slotDelegate.scale = 1.05
                onExited: slotDelegate.scale = 1.0
            }

            // 动画
            Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
        }
    }

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
            case "weapon":     return "\u2694\uFE0F"
            case "armor":      return "\uD83D\uDEE1\uFE0F"
            case "consumable": return "\uD83E\uDDEA"
            case "material":   return "\uD83D\uDD27"
            case "quest":      return "\uD83D\uDCDC"
            case "misc":       return "\uD83D\uDCE6"
            default:           return "\u2753"
        }
    }

    function getUsedSlots() {
        var count = 0
        for (var i = 0; i < gridModel.count; i++) {
            if (gridModel.get(i).slotItemId > 0) {
                count++
            }
        }
        return count
    }

    function setSlotData(index, data) {
        if (index < 0 || index >= gridModel.count) return

        if (data && data.id > 0) {
            gridModel.set(index, {
                "slotItemId": data.id,
                "slotItemName": data.name || "",
                "slotItemIcon": data.icon || "",
                "slotItemQuantity": data.quantity || 1,
                "slotItemQuality": data.quality || 0,
                "slotItemType": data.type || "",
                "slotItemDescription": data.description || ""
            })
        } else {
            clearSlot(index)
        }
    }

    function getSlotData(index) {
        if (index < 0 || index >= gridModel.count) return null
        return gridModel.get(index)
    }

    function clearSlot(index) {
        if (index < 0 || index >= gridModel.count) return
        gridModel.set(index, {
            "slotItemId": 0,
            "slotItemName": "",
            "slotItemIcon": "",
            "slotItemQuantity": 0,
            "slotItemQuality": 0,
            "slotItemType": "",
            "slotItemDescription": ""
        })
    }

    function clearAll() {
        for (var i = 0; i < gridModel.count; i++) {
            clearSlot(i)
        }
        selectedIndex = -1
    }

    function swapSlots(fromIndex, toIndex) {
        if (fromIndex < 0 || fromIndex >= gridModel.count) return
        if (toIndex < 0 || toIndex >= gridModel.count) return
        if (fromIndex === toIndex) return

        var fromData = {
            slotItemId: gridModel.get(fromIndex).slotItemId,
            slotItemName: gridModel.get(fromIndex).slotItemName,
            slotItemIcon: gridModel.get(fromIndex).slotItemIcon,
            slotItemQuantity: gridModel.get(fromIndex).slotItemQuantity,
            slotItemQuality: gridModel.get(fromIndex).slotItemQuality,
            slotItemType: gridModel.get(fromIndex).slotItemType,
            slotItemDescription: gridModel.get(fromIndex).slotItemDescription
        }

        gridModel.set(fromIndex, {
            "slotItemId": gridModel.get(toIndex).slotItemId,
            "slotItemName": gridModel.get(toIndex).slotItemName,
            "slotItemIcon": gridModel.get(toIndex).slotItemIcon,
            "slotItemQuantity": gridModel.get(toIndex).slotItemQuantity,
            "slotItemQuality": gridModel.get(toIndex).slotItemQuality,
            "slotItemType": gridModel.get(toIndex).slotItemType,
            "slotItemDescription": gridModel.get(toIndex).slotItemDescription
        })

        gridModel.set(toIndex, {
            "slotItemId": fromData.slotItemId,
            "slotItemName": fromData.slotItemName,
            "slotItemIcon": fromData.slotItemIcon,
            "slotItemQuantity": fromData.slotItemQuantity,
            "slotItemQuality": fromData.slotItemQuality,
            "slotItemType": fromData.slotItemType,
            "slotItemDescription": fromData.slotItemDescription
        })
    }
}
