// ============================================================================
// 《幻境传说》游戏框架 - 物品槽组件
// ============================================================================
// 文件名: ItemSlot.qml
// 说明: 通用物品槽位组件，用于背包、装备栏、商店等场景的物品显示
// 作者: 彭承康
// 创建时间: 2026-02-18
// 版本: v1.0.0
//
// 功能描述:
// - 物品图标和数量显示
// - 拖拽操作支持
// - 物品品质颜色区分
// - 右键菜单功能
// - 物品详情提示显示
//
// 使用场景:
// - 背包系统的物品槽
// - 装备栏的装备槽
// - 商店界面的商品槽
// - 交易界面的物品槽
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * @brief 物品槽位组件
 * @details 提供标准化的物品槽位显示，支持拖拽、右键菜单、品质显示等功能
 * 
 * 主要功能:
 * - 物品图标和信息显示
 * - 拖拽操作和放置检测
 * - 物品品质边框效果
 * - 右键上下文菜单
 * - 物品数量和堆叠显示
 * 
 * 使用示例:
 * @code
 * ItemSlot {
 *     itemId: 1001
 *     itemIcon: "sword.png"
 *     itemName: "铁剑"
 *     itemQuantity: 1
 *     itemQuality: 2
 *     onItemClicked: {
 *         // 处理物品点击
 *     }
 * }
 * @endcode
 */
Rectangle {
    id: itemSlot
    
    // ------------------------------------------------------------------------
    // 组件尺寸和外观
    // ------------------------------------------------------------------------
    
    width: 64
    height: 64
    radius: 8
    
    // 根据物品品质设置边框颜色
    border.color: getQualityColor(itemQuality)
    border.width: hasItem ? 2 : 1
    
    // 背景颜色
    color: {
        if (dragArea.containsDrag) return "#3498db"
        if (dragArea.pressed) return "#2980b9"
        if (hasItem) return "#34495e"
        return "#7f8c8d"
    }
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 物品ID（0表示空槽位）
    property int itemId: 0
    
    /// 物品图标路径
    property string itemIcon: ""
    
    /// 物品名称
    property string itemName: ""
    
    /// 物品数量
    property int itemQuantity: 0
    
    /// 物品品质等级（0-5，对应不同颜色）
    property int itemQuality: 0
    
    /// 物品类型
    property string itemType: ""
    
    /// 物品描述
    property string itemDescription: ""
    
    /// 是否可拖拽
    property bool draggable: true
    
    /// 是否可接受拖拽
    property bool droppable: true
    
    /// 是否显示数量
    property bool showQuantity: true
    
    /// 槽位类型（用于限制可放置的物品类型）
    property string slotType: "any"
    
    /// 是否有物品
    readonly property bool hasItem: itemId > 0
    
    /// 是否被选中
    property bool selected: false
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 物品被点击信号
    /// @param itemId 物品ID
    /// @param button 鼠标按键（1=左键，2=右键）
    signal itemClicked(int itemId, int button)
    
    /// 物品被双击信号
    /// @param itemId 物品ID
    signal itemDoubleClicked(int itemId)
    
    /// 物品拖拽开始信号
    /// @param itemId 物品ID
    signal dragStarted(int itemId)
    
    /// 物品拖拽结束信号
    /// @param itemId 物品ID
    /// @param success 是否成功放置
    signal dragFinished(int itemId, bool success)
    
    /// 物品被放置信号
    /// @param sourceItemId 源物品ID
    /// @param targetSlot 目标槽位
    signal itemDropped(int sourceItemId, var targetSlot)
    
    /// 显示物品详情信号
    /// @param itemId 物品ID
    /// @param x 显示位置X
    /// @param y 显示位置Y
    signal showItemTooltip(int itemId, real x, real y)
    
    /// 隐藏物品详情信号
    signal hideItemTooltip()
    
    // ------------------------------------------------------------------------
    // 主要内容显示
    // ------------------------------------------------------------------------
    
    // 物品图标
    Image {
        id: itemImage
        anchors.centerIn: parent
        width: parent.width - 8
        height: parent.height - 8
        source: itemSlot.hasItem ? itemSlot.itemIcon : ""
        fillMode: Image.PreserveAspectFit
        visible: itemSlot.hasItem
        
        // 图标加载失败时显示默认图标
        onStatusChanged: {
            if (status === Image.Error && itemSlot.hasItem) {
                source = "qrc:/resources/images/default_item.png"
            }
        }
        
        // 拖拽时的透明度效果
        opacity: dragArea.drag.active ? 0.5 : 1.0
        
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }
    
    // 默认物品图标（当没有图片时显示）
    Text {
        anchors.centerIn: parent
        text: getDefaultIcon(itemSlot.itemType)
        font.pixelSize: 32
        color: "#bdc3c7"
        visible: itemSlot.hasItem && (itemImage.status === Image.Error || itemSlot.itemIcon === "")
    }
    
    // 物品数量显示
    Rectangle {
        id: quantityBadge
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 2
        anchors.rightMargin: 2
        width: quantityText.width + 6
        height: quantityText.height + 4
        radius: 3
        color: "#2c3e50"
        border.color: "#34495e"
        border.width: 1
        visible: itemSlot.hasItem && itemSlot.showQuantity && itemSlot.itemQuantity > 1
        
        Text {
            id: quantityText
            anchors.centerIn: parent
            text: itemSlot.itemQuantity > 999 ? "999+" : itemSlot.itemQuantity.toString()
            color: "white"
            font.pixelSize: 10
            font.bold: true
        }
    }
    
    // 品质光效（高品质物品的特效）
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: getQualityColor(itemSlot.itemQuality)
        border.width: 1
        opacity: 0.3
        visible: itemSlot.hasItem && itemSlot.itemQuality >= 3
        
        // 闪烁动画
        SequentialAnimation on opacity {
            running: visible
            loops: Animation.Infinite
            NumberAnimation { to: 0.1; duration: 1500 }
            NumberAnimation { to: 0.5; duration: 1500 }
        }
    }
    
    // 选中状态指示器
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: "#f39c12"
        border.width: 3
        visible: itemSlot.selected
        
        // 选中动画
        SequentialAnimation on border.width {
            running: visible
            loops: Animation.Infinite
            NumberAnimation { to: 2; duration: 800 }
            NumberAnimation { to: 4; duration: 800 }
        }
    }
    
    // 空槽位提示
    Text {
        anchors.centerIn: parent
        text: "+"
        font.pixelSize: 24
        color: "#95a5a6"
        visible: !itemSlot.hasItem
        opacity: 0.5
    }
    
    // ------------------------------------------------------------------------
    // 鼠标交互区域
    // ------------------------------------------------------------------------
    
    MouseArea {
        id: dragArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        // 拖拽属性
        drag.target: itemSlot.hasItem && itemSlot.draggable ? dragProxy : null
        drag.axis: Drag.XAndYAxis
        
        // 鼠标悬停效果
        onEntered: {
            if (itemSlot.hasItem) {
                itemSlot.scale = 1.05
                // 显示物品详情提示
                var globalPos = mapToItem(itemSlot.parent, mouseX, mouseY)
                itemSlot.showItemTooltip(itemSlot.itemId, globalPos.x, globalPos.y)
            }
        }
        
        onExited: {
            itemSlot.scale = 1.0
            itemSlot.hideItemTooltip()
        }
        
        // 鼠标点击处理
        onClicked: function(mouse) {
            if (itemSlot.hasItem) {
                itemSlot.itemClicked(itemSlot.itemId, mouse.button)
                console.log("物品被点击:", itemSlot.itemName, "按键:", mouse.button)
            }
        }
        
        // 双击处理
        onDoubleClicked: {
            if (itemSlot.hasItem) {
                itemSlot.itemDoubleClicked(itemSlot.itemId)
                console.log("物品被双击:", itemSlot.itemName)
            }
        }
        
        // 拖拽开始
        onPressed: function(mouse) {
            if (itemSlot.hasItem && itemSlot.draggable && mouse.button === Qt.LeftButton) {
                dragProxy.x = itemSlot.x
                dragProxy.y = itemSlot.y
                dragProxy.visible = true
                itemSlot.dragStarted(itemSlot.itemId)
            }
        }
        
        // 拖拽结束
        onReleased: {
            if (dragProxy.visible) {
                dragProxy.visible = false
                var success = dragProxy.Drag.target !== null
                itemSlot.dragFinished(itemSlot.itemId, success)
                
                if (!success) {
                    // 拖拽失败，物品回到原位置
                    console.log("拖拽失败，物品回到原位置")
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 拖拽代理对象
    // ------------------------------------------------------------------------
    
    Rectangle {
        id: dragProxy
        width: itemSlot.width
        height: itemSlot.height
        radius: itemSlot.radius
        color: itemSlot.color
        border.color: itemSlot.border.color
        border.width: itemSlot.border.width
        visible: false
        opacity: 0.8
        z: 1000
        
        Drag.active: dragArea.drag.active
        Drag.source: itemSlot
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        
        // 拖拽的物品信息
        Drag.keys: [itemSlot.itemType, "item"]
        Drag.mimeData: {
            "application/x-item-id": itemSlot.itemId,
            "application/x-item-type": itemSlot.itemType,
            "application/x-source-slot": itemSlot
        }
        
        // 拖拽时显示的物品图标
        Image {
            anchors.centerIn: parent
            width: parent.width - 8
            height: parent.height - 8
            source: itemSlot.itemIcon
            fillMode: Image.PreserveAspectFit
        }
        
        // 拖拽时的数量显示
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.bottomMargin: 2
            anchors.rightMargin: 2
            width: dragQuantityText.width + 6
            height: dragQuantityText.height + 4
            radius: 3
            color: "#2c3e50"
            visible: itemSlot.showQuantity && itemSlot.itemQuantity > 1
            
            Text {
                id: dragQuantityText
                anchors.centerIn: parent
                text: itemSlot.itemQuantity.toString()
                color: "white"
                font.pixelSize: 10
                font.bold: true
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 拖拽放置区域
    // ------------------------------------------------------------------------
    
    DropArea {
        anchors.fill: parent
        enabled: itemSlot.droppable
        
        keys: [itemSlot.slotType, "item"]
        
        onEntered: function(drag) {
            console.log("物品拖拽进入槽位")
            itemSlot.color = "#3498db"
        }
        
        onExited: {
            console.log("物品拖拽离开槽位")
            itemSlot.color = itemSlot.hasItem ? "#34495e" : "#7f8c8d"
        }
        
        onDropped: function(drop) {
            console.log("物品被放置到槽位")
            var sourceItemId = drop.getDataAsString("application/x-item-id")
            var sourceSlot = drop.getDataAsString("application/x-source-slot")
            
            itemSlot.itemDropped(parseInt(sourceItemId), itemSlot)
            itemSlot.color = itemSlot.hasItem ? "#34495e" : "#7f8c8d"
            
            drop.acceptProposedAction()
        }
    }
    
    // ------------------------------------------------------------------------
    // 动画效果
    // ------------------------------------------------------------------------
    
    // 缩放动画
    Behavior on scale {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }
    
    // 颜色变化动画
    Behavior on color {
        ColorAnimation { duration: 300 }
    }
    
    // 边框颜色动画
    Behavior on border.color {
        ColorAnimation { duration: 200 }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化和私有函数
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("ItemSlot 组件初始化完成")
    }
    
    /**
     * @brief 根据物品品质获取边框颜色
     * @param quality 品质等级（0-5）
     * @return 颜色值
     */
    function getQualityColor(quality) {
        switch(quality) {
            case 0: return "#95a5a6"  // 灰色 - 垃圾
            case 1: return "#ffffff"  // 白色 - 普通
            case 2: return "#2ecc71"  // 绿色 - 优秀
            case 3: return "#3498db"  // 蓝色 - 稀有
            case 4: return "#9b59b6"  // 紫色 - 史诗
            case 5: return "#f39c12"  // 橙色 - 传说
            default: return "#95a5a6"
        }
    }
    
    /**
     * @brief 根据物品类型获取默认图标
     * @param type 物品类型
     * @return 图标字符
     */
    function getDefaultIcon(type) {
        switch(type) {
            case "weapon": return "⚔️"
            case "armor": return "🛡️"
            case "consumable": return "🧪"
            case "material": return "🔧"
            case "quest": return "📜"
            case "misc": return "📦"
            default: return "❓"
        }
    }
    
    /**
     * @brief 设置物品数据
     * @param data 物品数据对象
     */
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
    
    /**
     * @brief 清空槽位
     */
    function clearItem() {
        itemId = 0
        itemIcon = ""
        itemName = ""
        itemQuantity = 0
        itemQuality = 0
        itemType = ""
        itemDescription = ""
        selected = false
    }
    
    /**
     * @brief 检查是否可以放置指定类型的物品
     * @param type 物品类型
     * @return 是否可以放置
     */
    function canAcceptItemType(type) {
        if (slotType === "any") return true
        return slotType === type
    }
    
    /**
     * @brief 播放物品获得动画
     */
    function playItemGainAnimation() {
        // 创建获得物品的动画效果
        var animation = Qt.createQmlObject('
            import QtQuick 2.15
            SequentialAnimation {
                NumberAnimation { target: itemSlot; property: "scale"; to: 1.3; duration: 200 }
                NumberAnimation { target: itemSlot; property: "scale"; to: 1.0; duration: 200 }
                ScriptAction { script: animation.destroy() }
            }
        ', itemSlot)
        animation.start()
    }
}
