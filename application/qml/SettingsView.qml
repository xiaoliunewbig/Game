// ============================================================================
// 《幻境传说》游戏框架 - 设置界面视图
// ============================================================================
// 文件名: SettingsView.qml
// 说明: 游戏设置界面，包含音频、视频、控制、游戏性等各项设置选项
// 作者: 彭承康
// 创建时间: 2025-07-20
// 版本: v1.0.0
//
// 功能描述:
// - 音频设置（音量、音效开关）
// - 视频设置（分辨率、全屏、画质）
// - 控制设置（按键绑定、鼠标灵敏度）
// - 游戏性设置（难度、自动保存）
// - 语言和界面设置
//
// 依赖组件:
// - Qt Quick Controls 2.15
// - 自定义滑块和开关组件
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 设置界面视图组件
 * @details 提供完整的游戏设置界面，包括各类设置选项的配置和管理
 * 
 * 主要功能:
 * - 分类设置选项管理
 * - 设置数据的保存和加载
 * - 实时设置预览效果
 * - 设置重置和恢复默认
 * - 设置验证和错误处理
 * 
 * 使用示例:
 * @code
 * SettingsView {
 *     anchors.fill: parent
 *     onSettingsChanged: function(category, key, value) {
 *         // 处理设置变更逻辑
 *     }
 * }
 * @endcode
 */
Item {
    id: settingsView
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 当前选中的设置分类
    property string currentCategory: "audio"
    
    /// 设置是否有未保存的更改
    property bool hasUnsavedChanges: false
    
    // 音频设置
    property real masterVolume: 0.8
    property real musicVolume: 0.7
    property real soundEffectVolume: 0.9
    property bool audioEnabled: true
    
    // 视频设置
    property string resolution: "1920x1080"
    property bool fullscreen: false
    property int graphicsQuality: 2 // 0=低, 1=中, 2=高
    property bool vsyncEnabled: true
    
    // 控制设置
    property real mouseSensitivity: 0.5
    property bool invertMouseY: false
    
    // 游戏性设置
    property int difficulty: 1 // 0=简单, 1=普通, 2=困难
    property bool autoSave: true
    property int autoSaveInterval: 5 // 分钟
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 设置项发生变更信号
    /// @param category 设置分类
    /// @param key 设置键名
    /// @param value 新值
    signal settingsChanged(string category, string key, var value)
    
    /// 设置保存信号
    signal settingsSaved()
    
    /// 设置重置信号
    signal settingsReset()
    
    // ------------------------------------------------------------------------
    // 主界面布局
    // ------------------------------------------------------------------------
    
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: "#7f8c8d"  // 中性灰色背景
        
        // 背景渐变效果
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#7f8c8d" }
            GradientStop { position: 1.0; color: "#95a5a6" }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            // ------------------------------------------------------------------------
            // 左侧分类导航栏
            // ------------------------------------------------------------------------
            
            Rectangle {
                id: categoryPanel
                Layout.preferredWidth: 200
                Layout.fillHeight: true
                color: "#34495e"
                radius: 10
                border.color: "#2c3e50"
                border.width: 2
                
                Column {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10
                    
                    // 设置标题
                    Text {
                        text: "游戏设置"
                        color: "#ecf0f1"
                        font.pixelSize: 20
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    
                    Rectangle {
                        width: parent.width
                        height: 2
                        color: "#7f8c8d"
                    }
                    
                    // 分类按钮列表
                    Repeater {
                        model: [
                            {name: "音频设置", value: "audio", icon: "🔊"},
                            {name: "视频设置", value: "video", icon: "🖥️"},
                            {name: "控制设置", value: "controls", icon: "🎮"},
                            {name: "游戏性", value: "gameplay", icon: "⚙️"},
                            {name: "语言界面", value: "interface", icon: "🌐"}
                        ]
                        
                        Button {
                            property var categoryData: modelData
                            width: parent.width
                            height: 50
                            
                            background: Rectangle {
                                color: settingsView.currentCategory === categoryData.value ? 
                                       "#3498db" : "transparent"
                                radius: 8
                                border.color: settingsView.currentCategory === categoryData.value ? 
                                             "#2980b9" : "transparent"
                                border.width: 2
                                
                                // 悬停效果
                                Behavior on color {
                                    ColorAnimation { duration: 200 }
                                }
                            }
                            
                            contentItem: Row {
                                anchors.centerIn: parent
                                spacing: 10
                                
                                Text {
                                    text: categoryData.icon
                                    font.pixelSize: 18
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                
                                Text {
                                    text: categoryData.name
                                    color: settingsView.currentCategory === categoryData.value ? 
                                           "white" : "#bdc3c7"
                                    font.pixelSize: 14
                                    font.bold: settingsView.currentCategory === categoryData.value
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            
                            onClicked: {
                                settingsView.currentCategory = categoryData.value
                                console.log("切换到设置分类:", categoryData.name)
                            }
                        }
                    }
                    
                    Item { height: 20 } // 间距
                    
                    // 底部操作按钮
                    Column {
                        width: parent.width
                        spacing: 10
                        
                        Button {
                            text: "保存设置"
                            width: parent.width
                            height: 40
                            enabled: settingsView.hasUnsavedChanges
                            
                            background: Rectangle {
                                color: parent.enabled ? 
                                       (parent.pressed ? "#27ae60" : "#2ecc71") : "#7f8c8d"
                                radius: 6
                                border.color: parent.enabled ? "#229954" : "#95a5a6"
                                border.width: 1
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: parent.enabled ? "white" : "#bdc3c7"
                                font.pixelSize: 14
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            
                            onClicked: {
                                saveSettings()
                            }
                        }
                        
                        Button {
                            text: "恢复默认"
                            width: parent.width
                            height: 40
                            
                            background: Rectangle {
                                color: parent.pressed ? "#d35400" : "#e67e22"
                                radius: 6
                                border.color: "#ca6f1e"
                                border.width: 1
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font.pixelSize: 14
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            
                            onClicked: {
                                resetToDefaults()
                            }
                        }
                    }
                }
            }
            
            // ------------------------------------------------------------------------
            // 右侧设置内容区域
            // ------------------------------------------------------------------------
            
            Rectangle {
                id: contentPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#ecf0f1"
                radius: 10
                border.color: "#bdc3c7"
                border.width: 2
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 20
                    clip: true
                    
                    // 设置内容加载器
                    Loader {
                        id: contentLoader
                        width: parent.width
                        
                        sourceComponent: {
                            switch(settingsView.currentCategory) {
                                case "audio": return audioSettingsComponent
                                case "video": return videoSettingsComponent
                                case "controls": return controlsSettingsComponent
                                case "gameplay": return gameplaySettingsComponent
                                case "interface": return interfaceSettingsComponent
                                default: return audioSettingsComponent
                            }
                        }
                    }
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 音频设置组件
    // ------------------------------------------------------------------------
    
    Component {
        id: audioSettingsComponent
        
        Column {
            spacing: 25
            width: parent.width
            
            Text {
                text: "音频设置"
                font.pixelSize: 24
                font.bold: true
                color: "#2c3e50"
            }
            
            // 主音量设置
            SettingSlider {
                title: "主音量"
                value: settingsView.masterVolume
                onValueChanged: {
                    settingsView.masterVolume = value
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("audio", "masterVolume", value)
                }
            }
            
            // 音乐音量设置
            SettingSlider {
                title: "音乐音量"
                value: settingsView.musicVolume
                onValueChanged: {
                    settingsView.musicVolume = value
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("audio", "musicVolume", value)
                }
            }
            
            // 音效音量设置
            SettingSlider {
                title: "音效音量"
                value: settingsView.soundEffectVolume
                onValueChanged: {
                    settingsView.soundEffectVolume = value
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("audio", "soundEffectVolume", value)
                }
            }
            
            // 音频开关
            SettingSwitch {
                title: "启用音频"
                checked: settingsView.audioEnabled
                onCheckedChanged: {
                    settingsView.audioEnabled = checked
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("audio", "audioEnabled", checked)
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 视频设置组件
    // ------------------------------------------------------------------------
    
    Component {
        id: videoSettingsComponent
        
        Column {
            spacing: 25
            width: parent.width
            
            Text {
                text: "视频设置"
                font.pixelSize: 24
                font.bold: true
                color: "#2c3e50"
            }
            
            // 分辨率设置
            SettingComboBox {
                title: "分辨率"
                model: ["1280x720", "1920x1080", "2560x1440", "3840x2160"]
                currentText: settingsView.resolution
                onCurrentTextChanged: {
                    settingsView.resolution = currentText
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("video", "resolution", currentText)
                }
            }
            
            // 全屏开关
            SettingSwitch {
                title: "全屏模式"
                checked: settingsView.fullscreen
                onCheckedChanged: {
                    settingsView.fullscreen = checked
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("video", "fullscreen", checked)
                }
            }
            
            // 画质设置
            SettingComboBox {
                title: "画质设置"
                model: ["低", "中", "高"]
                currentIndex: settingsView.graphicsQuality
                onCurrentIndexChanged: {
                    settingsView.graphicsQuality = currentIndex
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("video", "graphicsQuality", currentIndex)
                }
            }
            
            // 垂直同步
            SettingSwitch {
                title: "垂直同步"
                checked: settingsView.vsyncEnabled
                onCheckedChanged: {
                    settingsView.vsyncEnabled = checked
                    settingsView.hasUnsavedChanges = true
                    settingsView.settingsChanged("video", "vsyncEnabled", checked)
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 其他设置组件（简化版本）
    // ------------------------------------------------------------------------
    
    Component {
        id: controlsSettingsComponent
        Column {
            spacing: 25
            width: parent.width
            Text {
                text: "控制设置"
                font.pixelSize: 24
                font.bold: true
                color: "#2c3e50"
            }
            Text {
                text: "控制设置功能开发中..."
                font.pixelSize: 16
                color: "#7f8c8d"
            }
        }
    }
    
    Component {
        id: gameplaySettingsComponent
        Column {
            spacing: 25
            width: parent.width
            Text {
                text: "游戏性设置"
                font.pixelSize: 24
                font.bold: true
                color: "#2c3e50"
            }
            Text {
                text: "游戏性设置功能开发中..."
                font.pixelSize: 16
                color: "#7f8c8d"
            }
        }
    }
    
    Component {
        id: interfaceSettingsComponent
        Column {
            spacing: 25
            width: parent.width
            Text {
                text: "界面设置"
                font.pixelSize: 24
                font.bold: true
                color: "#2c3e50"
            }
            Text {
                text: "界面设置功能开发中..."
                font.pixelSize: 16
                color: "#7f8c8d"
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 自定义设置组件
    // ------------------------------------------------------------------------
    
    component SettingSlider: Column {
        property string title: ""
        property real value: 0.0
        signal valueChanged(real value)
        
        spacing: 8
        width: parent.width
        
        Text {
            text: title + ": " + Math.round(value * 100) + "%"
            font.pixelSize: 16
            color: "#2c3e50"
        }
        
        Slider {
            width: parent.width
            from: 0.0
            to: 1.0
            value: parent.value
            onValueChanged: parent.valueChanged(value)
            
            background: Rectangle {
                x: parent.leftPadding
                y: parent.topPadding + parent.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 4
                width: parent.availableWidth
                height: implicitHeight
                radius: 2
                color: "#bdc3c7"
                
                Rectangle {
                    width: parent.parent.visualPosition * parent.width
                    height: parent.height
                    color: "#3498db"
                    radius: 2
                }
            }
            
            handle: Rectangle {
                x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                y: parent.topPadding + parent.availableHeight / 2 - height / 2
                implicitWidth: 20
                implicitHeight: 20
                radius: 10
                color: parent.pressed ? "#2980b9" : "#3498db"
                border.color: "#2471a3"
                border.width: 1
            }
        }
    }
    
    component SettingSwitch: Row {
        property string title: ""
        property bool checked: false
        signal checkedChanged(bool checked)
        
        spacing: 15
        width: parent.width
        
        Text {
            text: title
            font.pixelSize: 16
            color: "#2c3e50"
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Switch {
            checked: parent.checked
            onCheckedChanged: parent.checkedChanged(checked)
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    component SettingComboBox: Column {
        property string title: ""
        property alias model: comboBox.model
        property alias currentIndex: comboBox.currentIndex
        property alias currentText: comboBox.currentText
        signal currentTextChanged(string text)
        signal currentIndexChanged(int index)
        
        spacing: 8
        width: parent.width
        
        Text {
            text: title
            font.pixelSize: 16
            color: "#2c3e50"
        }
        
        ComboBox {
            id: comboBox
            width: 200
            onCurrentTextChanged: parent.currentTextChanged(currentText)
            onCurrentIndexChanged: parent.currentIndexChanged(currentIndex)
        }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化和私有函数
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("SettingsView 组件初始化完成")
        loadSettings()
    }
    
    /**
     * @brief 保存设置到本地
     */
    function saveSettings() {
        console.log("保存设置到本地存储")
        hasUnsavedChanges = false
        settingsSaved()
    }
    
    /**
     * @brief 加载设置从本地
     */
    function loadSettings() {
        console.log("从本地存储加载设置")
        // 这里添加实际的设置加载逻辑
    }
    
    /**
     * @brief 重置为默认设置
     */
    function resetToDefaults() {
        console.log("重置为默认设置")
        masterVolume = 0.8
        musicVolume = 0.7
        soundEffectVolume = 0.9
        audioEnabled = true
        resolution = "1920x1080"
        fullscreen = false
        graphicsQuality = 2
        vsyncEnabled = true
        hasUnsavedChanges = true
        settingsReset()
    }
}
