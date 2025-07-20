// ============================================================================
// 《幻境传说》游戏框架 - 背包界面视图
// ============================================================================
// 文件名: InventoryView.qml
// 说明: 游戏背包系统的主界面，管理物品显示、分类、使用等功能
// 作者: 彭承康
// 创建时间: 2025-07-20
// 版本: v1.0.0
//
// 功能描述:
// - 物品网格显示和管理
// - 物品分类和筛选功能
// - 物品详情查看和使用
// - 背包容量管理
// - 物品排序和搜索
//
// 依赖组件:
// - InventoryGrid: 物品网格布局
// - ItemCard: 物品卡片显示
// - ItemSlot: 物品槽位组件
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 背包界面视图组件
 * @details 提供完整的背包管理界面，包括物品展示、分类筛选、详情查看等功能
 * 
 * 主要功能:
 * - 物品网格化显示
 * - 按类型分类筛选
 * - 物品搜索功能
 * - 物品使用和丢弃
 * - 背包容量显示
 * 
 * 使用示例:
 * @code
 * InventoryView {
 *     anchors.fill: parent
 *     onItemUsed: function(itemId) {
 *         // 处理物品使用逻辑
 *     }
 * }
 * @endcode
 */
Item {
    id: inventoryView
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 背包最大容量
    property int maxCapacity: 50
    
    /// 当前物品数量
    property int currentItemCount: 0
    
    /// 当前选中的物品类型筛选
    property string selectedCategory: "all"
    
    /// 搜索关键词
    property string searchKeyword: ""
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 物品被使用信号
    /// @param itemId 物品ID
    signal itemUsed(int itemId)
    
    /// 物品被丢弃信号
    /// @param itemId 物品ID
    signal itemDiscarded(int itemId)
    
    /// 物品被选中信号
    /// @param itemId 物品ID
    signal itemSelected(int itemId)
    
    // ------------------------------------------------------------------------
    // 主界面布局
    // ------------------------------------------------------------------------
    
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: "#34495e"  // 深灰色背景
        
        // 背景渐变效果
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#34495e" }
            GradientStop { position: 1.0; color: "#2c3e50" }
        }
        
        // ------------------------------------------------------------------------
        // 顶部工具栏
        // ------------------------------------------------------------------------
        
        Rectangle {
            id: toolBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 80
            color: "#2c3e50"
            border.color: "#7f8c8d"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15
                
                // 背包标题
                Text {
                    text: "背包"
                    color: "#ecf0f1"
                    font.pixelSize: 24
                    font.bold: true
                }
                
                // 容量显示
                Text {
                    text: inventoryView.currentItemCount + "/" + inventoryView.maxCapacity
                    color: inventoryView.currentItemCount >= inventoryView.maxCapacity ? "#e74c3c" : "#95a5a6"
                    font.pixelSize: 16
                }
                
                Item { Layout.fillWidth: true } // 弹性空间
                
                // 搜索框
                TextField {
                    id: searchField
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 35
                    placeholderText: "搜索物品..."
                    color: "#2c3e50"
                    
                    background: Rectangle {
                        color: "#ecf0f1"
                        radius: 6
                        border.color: searchField.activeFocus ? "#3498db" : "#bdc3c7"
                        border.width: 2
                    }
                    
                    onTextChanged: {
                        inventoryView.searchKeyword = text
                        console.log("搜索关键词:", text)
                    }
                }
                
                // 排序按钮
                Button {
                    text: "排序"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 35
                    
                    background: Rectangle {
                        color: parent.pressed ? "#2980b9" : "#3498db"
                        radius: 6
                        border.color: "#2471a3"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("执行物品排序")
                        // 这里添加排序逻辑
                    }
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 分类筛选栏
        // ------------------------------------------------------------------------
        
        Rectangle {
            id: categoryBar
            anchors.top: toolBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "#2c3e50"
            border.color: "#7f8c8d"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                
                // 分类按钮组
                Repeater {
                    model: [
                        {name: "全部", value: "all", color: "#95a5a6"},
                        {name: "武器", value: "weapon", color: "#e74c3c"},
                        {name: "防具", value: "armor", color: "#3498db"},
                        {name: "消耗品", value: "consumable", color: "#2ecc71"},
                        {name: "材料", value: "material", color: "#f39c12"},
                        {name: "任务", value: "quest", color: "#9b59b6"}
                    ]
                    
                    Button {
                        property var categoryData: modelData
                        text: categoryData.name
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 40
                        
                        background: Rectangle {
                            color: inventoryView.selectedCategory === categoryData.value ? 
                                   categoryData.color : "#7f8c8d"
                            radius: 6
                            border.color: categoryData.color
                            border.width: inventoryView.selectedCategory === categoryData.value ? 2 : 1
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 12
                            font.bold: inventoryView.selectedCategory === categoryData.value
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            inventoryView.selectedCategory = categoryData.value
                            console.log("选择分类:", categoryData.name)
                        }
                    }
                }
                
                Item { Layout.fillWidth: true } // 弹性空间
            }
        }
        
        // ------------------------------------------------------------------------
        // 物品网格区域
        // ------------------------------------------------------------------------
        
        ScrollView {
            id: scrollView
            anchors.top: categoryBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            
            clip: true
            
            // 自定义滚动条样式
            ScrollBar.vertical: ScrollBar {
                active: true
                policy: ScrollBar.AsNeeded
                
                background: Rectangle {
                    color: "#7f8c8d"
                    radius: 6
                }
                
                contentItem: Rectangle {
                    color: "#bdc3c7"
                    radius: 6
                }
            }
            
            GridView {
                id: itemGrid
                anchors.fill: parent
                cellWidth: 90
                cellHeight: 110
                
                // 模拟物品数据 - 后续替换为实际数据绑定
                model: 30
                
                delegate: Rectangle {
                    width: 80
                    height: 100
                    color: "#7f8c8d"
                    radius: 8
                    border.color: "#95a5a6"
                    border.width: 1
                    
                    // 鼠标悬停效果
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        
                        onEntered: {
                            parent.color = "#95a5a6"
                            parent.scale = 1.05
                        }
                        
                        onExited: {
                            parent.color = "#7f8c8d"
                            parent.scale = 1.0
                        }
                        
                        onClicked: {
                            console.log("选中物品:", index)
                            inventoryView.itemSelected(index)
                        }
                        
                        onDoubleClicked: {
                            console.log("使用物品:", index)
                            inventoryView.itemUsed(index)
                        }
                    }
                    
                    // 缩放动画
                    Behavior on scale {
                        NumberAnimation { duration: 150 }
                    }
                    
                    // 颜色变化动画
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        
                        // 物品图标占位
                        Rectangle {
                            width: 48
                            height: 48
                            color: "#bdc3c7"
                            radius: 6
                            anchors.horizontalCenter: parent.horizontalCenter
                            
                            Text {
                                anchors.centerIn: parent
                                text: "📦"
                                font.pixelSize: 24
                            }
                        }
                        
                        // 物品名称
                        Text {
                            text: "物品" + (index + 1)
                            color: "#ecf0f1"
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                            elide: Text.ElideRight
                            width: 70
                            horizontalAlignment: Text.AlignHCenter
                        }
                        
                        // 物品数量
                        Text {
                            text: "x" + (Math.floor(Math.random() * 10) + 1)
                            color: "#95a5a6"
                            font.pixelSize: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 空背包提示
        // ------------------------------------------------------------------------
        
        Text {
            anchors.centerIn: scrollView
            text: "背包空空如也\n快去冒险收集物品吧！"
            color: "#7f8c8d"
            font.pixelSize: 18
            horizontalAlignment: Text.AlignHCenter
            visible: inventoryView.currentItemCount === 0
            
            // 淡入淡出动画
            SequentialAnimation on opacity {
                running: visible
                loops: Animation.Infinite
                NumberAnimation { to: 0.5; duration: 2000 }
                NumberAnimation { to: 1.0; duration: 2000 }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("InventoryView 组件初始化完成")
        currentItemCount = 15 // 模拟初始物品数量
    }
    
    // ------------------------------------------------------------------------
    // 私有函数
    // ------------------------------------------------------------------------
    
    /**
     * @brief 刷新物品显示
     * @details 根据当前筛选条件重新加载物品列表
     */
    function refreshItems() {
        console.log("刷新物品列表，分类:", selectedCategory, "搜索:", searchKeyword)
        // 这里添加实际的数据刷新逻辑
    }
    
    /**
     * @brief 添加物品到背包
     * @param itemId 物品ID
     * @param quantity 数量
     * @return 是否添加成功
     */
    function addItem(itemId, quantity) {
        if (currentItemCount >= maxCapacity) {
            console.log("背包已满，无法添加物品")
            return false
        }
        
        currentItemCount += quantity
        console.log("添加物品:", itemId, "数量:", quantity)
        return true
    }
    
    /**
     * @brief 从背包移除物品
     * @param itemId 物品ID
     * @param quantity 数量
     * @return 是否移除成功
     */
    function removeItem(itemId, quantity) {
        if (currentItemCount >= quantity) {
            currentItemCount -= quantity
            console.log("移除物品:", itemId, "数量:", quantity)
            return true
        }
        return false
    }
}
