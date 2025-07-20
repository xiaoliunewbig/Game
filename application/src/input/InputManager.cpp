/*
 * 文件名: InputManager.cpp
 * 说明: 输入管理器实现
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 * 
 * 本文件实现了游戏的输入管理系统，负责处理键盘和鼠标输入，
 * 提供键位绑定功能，并将输入转换为游戏动作信号。
 * 
 * 主要功能：
 * - 键盘事件处理和状态跟踪
 * - 鼠标事件处理和状态跟踪
 * - 可配置的键位绑定系统
 * - 输入状态查询接口
 * - 游戏动作信号发送
 */

#include "input/InputManager.h"
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 * 
 * 初始化输入管理器，设置默认键位绑定
 */
InputManager::InputManager(QObject *parent)
    : QObject(parent)
    , m_inputEnabled(true)
{
    initializeDefaultKeyBindings();
}

/**
 * @brief 析构函数
 * 
 * 清理资源，输入管理器销毁时自动调用
 */
InputManager::~InputManager() = default;

/**
 * @brief 处理键盘按下事件
 * @param event 键盘事件对象
 * 
 * 当用户按下键盘按键时调用此方法，会：
 * 1. 记录按键状态到m_pressedKeys集合
 * 2. 查找对应的游戏动作
 * 3. 发送相应的信号
 */
void InputManager::handleKeyPress(QKeyEvent *event)
{
    if (!m_inputEnabled || !event) {
        return;
    }
    
    int key = event->key();
    
    // 记录按键状态 - 用于后续的isKeyPressed查询
    m_pressedKeys.insert(key);
    
    // 查找对应的游戏动作
    QString action = getActionForKey(key);
    if (!action.isEmpty()) {
        qDebug() << "InputManager: 触发动作" << action << "键位:" << key;
        emit actionTriggered(action);
        
        // 发送特定动作信号 - 方便UI组件直接连接
        emitSpecificActionSignal(action);
    }
    
    // 发送通用按键信号 - 用于调试或特殊处理
    emit keyPressed(key);
}

/**
 * @brief 处理键盘释放事件
 * @param event 键盘事件对象
 * 
 * 当用户释放键盘按键时调用此方法，会：
 * 1. 从m_pressedKeys集合中移除按键
 * 2. 发送动作释放信号
 */
void InputManager::handleKeyRelease(QKeyEvent *event)
{
    if (!m_inputEnabled || !event) {
        return;
    }
    
    int key = event->key();
    
    // 移除按键状态
    m_pressedKeys.remove(key);
    
    // 查找对应的游戏动作
    QString action = getActionForKey(key);
    if (!action.isEmpty()) {
        emit actionReleased(action);
    }
    
    // 发送通用按键释放信号
    emit keyReleased(key);
}

/**
 * @brief 处理鼠标按下事件
 * @param event 鼠标事件对象
 * 
 * 处理鼠标按键按下，记录按键状态和位置信息
 */
void InputManager::handleMousePress(QMouseEvent *event)
{
    if (!m_inputEnabled || !event) {
        return;
    }
    
    Qt::MouseButton button = event->button();
    QPoint position = event->pos();
    
    // 记录鼠标按键状态
    m_pressedMouseButtons.insert(button);
    
    qDebug() << "InputManager: 鼠标按下" << button << "位置:" << position;
    
    // 发送鼠标按下信号
    emit mousePressed(button, position);
    
    // 处理特定鼠标动作 - 左右键分别处理
    if (button == Qt::LeftButton) {
        emit leftMousePressed(position);
    } else if (button == Qt::RightButton) {
        emit rightMousePressed(position);
    }
}

/**
 * @brief 处理鼠标释放事件
 * @param event 鼠标事件对象
 * 
 * 处理鼠标按键释放，清除按键状态
 */
void InputManager::handleMouseRelease(QMouseEvent *event)
{
    if (!m_inputEnabled || !event) {
        return;
    }
    
    Qt::MouseButton button = event->button();
    QPoint position = event->pos();
    
    // 移除鼠标按键状态
    m_pressedMouseButtons.remove(button);
    
    // 发送鼠标释放信号
    emit mouseReleased(button, position);
    
    // 处理特定鼠标动作
    if (button == Qt::LeftButton) {
        emit leftMouseReleased(position);
    } else if (button == Qt::RightButton) {
        emit rightMouseReleased(position);
    }
}

/**
 * @brief 处理鼠标移动事件
 * @param event 鼠标事件对象
 * 
 * 处理鼠标移动，如果有按键按下则视为拖拽
 */
void InputManager::handleMouseMove(QMouseEvent *event)
{
    if (!m_inputEnabled || !event) {
        return;
    }
    
    QPoint position = event->pos();
    
    // 发送鼠标移动信号
    emit mouseMoved(position);
    
    // 如果有按键按下，发送拖拽信号
    if (!m_pressedMouseButtons.isEmpty()) {
        emit mouseDragged(position);
    }
}

/**
 * @brief 检查指定按键是否被按下
 * @param key 按键代码
 * @return 如果按键被按下返回true，否则返回false
 * 
 * 用于查询当前按键状态，常用于连续输入检测
 */
bool InputManager::isKeyPressed(int key) const
{
    return m_pressedKeys.contains(key);
}

/**
 * @brief 检查指定鼠标按键是否被按下
 * @param button 鼠标按键
 * @return 如果按键被按下返回true，否则返回false
 */
bool InputManager::isMouseButtonPressed(Qt::MouseButton button) const
{
    return m_pressedMouseButtons.contains(button);
}

/**
 * @brief 检查指定游戏动作是否被激活
 * @param action 游戏动作名称
 * @return 如果动作对应的按键被按下返回true，否则返回false
 * 
 * 这是更高级的查询接口，通过动作名称而不是具体按键查询
 */
bool InputManager::isActionPressed(const QString &action) const
{
    int key = m_keyBindings.value(action, -1);
    return key != -1 && isKeyPressed(key);
}

/**
 * @brief 设置键位绑定
 * @param action 游戏动作名称
 * @param key 对应的按键代码
 * 
 * 设置或更新动作与按键的绑定关系。如果按键已绑定到其他动作，
 * 会先解除旧的绑定关系。
 */
void InputManager::setKeyBinding(const QString &action, int key)
{
    // 移除旧的绑定 - 确保一个按键只绑定一个动作
    auto it = m_keyBindings.begin();
    while (it != m_keyBindings.end()) {
        if (it.value() == key) {
            it = m_keyBindings.erase(it);
        } else {
            ++it;
        }
    }
    
    // 设置新的绑定
    m_keyBindings[action] = key;
    
    qDebug() << "InputManager: 设置键位绑定" << action << "=" << key;
    emit keyBindingChanged(action, key);
}

/**
 * @brief 获取动作对应的按键
 * @param action 游戏动作名称
 * @return 对应的按键代码，如果未绑定返回-1
 */
int InputManager::getKeyBinding(const QString &action) const
{
    return m_keyBindings.value(action, -1);
}

/**
 * @brief 获取所有键位绑定
 * @return 包含所有动作-按键绑定的映射表
 * 
 * 用于保存配置或显示键位设置界面
 */
QMap<QString, int> InputManager::getAllKeyBindings() const
{
    return m_keyBindings;
}

/**
 * @brief 设置输入是否启用
 * @param enabled 是否启用输入
 * 
 * 当禁用输入时，会清空所有按键状态，防止按键"卡住"
 */
void InputManager::setInputEnabled(bool enabled)
{
    if (m_inputEnabled != enabled) {
        m_inputEnabled = enabled;
        
        // 如果禁用输入，清空所有按键状态
        if (!enabled) {
            m_pressedKeys.clear();
            m_pressedMouseButtons.clear();
        }
        
        qDebug() << "InputManager: 输入" << (enabled ? "启用" : "禁用");
        emit inputEnabledChanged(enabled);
    }
}

/**
 * @brief 获取输入是否启用
 * @return 输入启用状态
 */
bool InputManager::isInputEnabled() const
{
    return m_inputEnabled;
}

/**
 * @brief 初始化默认键位绑定
 * 
 * 设置游戏的默认键位配置，包括移动、战斗、界面等各类操作
 */
void InputManager::initializeDefaultKeyBindings()
{
    // 移动控制 - WASD标准配置
    m_keyBindings["move_up"] = Qt::Key_W;
    m_keyBindings["move_down"] = Qt::Key_S;
    m_keyBindings["move_left"] = Qt::Key_A;
    m_keyBindings["move_right"] = Qt::Key_D;
    
    // 战斗控制
    m_keyBindings["attack"] = Qt::Key_Space;      // 攻击
    m_keyBindings["defend"] = Qt::Key_Shift;      // 防御
    
    // 技能快捷键 - 数字键1-4
    m_keyBindings["skill_1"] = Qt::Key_1;
    m_keyBindings["skill_2"] = Qt::Key_2;
    m_keyBindings["skill_3"] = Qt::Key_3;
    m_keyBindings["skill_4"] = Qt::Key_4;
    
    // 界面控制
    m_keyBindings["inventory"] = Qt::Key_I;       // 背包
    m_keyBindings["character"] = Qt::Key_C;       // 角色面板
    m_keyBindings["menu"] = Qt::Key_Escape;       // 菜单
    m_keyBindings["map"] = Qt::Key_M;             // 地图
    
    // 系统控制
    m_keyBindings["pause"] = Qt::Key_P;           // 暂停
    m_keyBindings["screenshot"] = Qt::Key_F12;    // 截图
    
    qDebug() << "InputManager: 初始化默认键位绑定完成";
}

/**
 * @brief 根据按键查找对应的动作
 * @param key 按键代码
 * @return 对应的动作名称，如果未找到返回空字符串
 * 
 * 这是内部辅助方法，用于反向查找键位绑定
 */
QString InputManager::getActionForKey(int key) const
{
    for (auto it = m_keyBindings.begin(); it != m_keyBindings.end(); ++it) {
        if (it.value() == key) {
            return it.key();
        }
    }
    return QString();
}

/**
 * @brief 发送特定动作信号
 * @param action 动作名称
 * 
 * 根据动作名称发送对应的具体信号，方便UI组件直接连接
 * 而不需要解析通用的actionTriggered信号
 */
void InputManager::emitSpecificActionSignal(const QString &action)
{
    // 移动动作
    if (action == "move_up") {
        emit moveUpPressed();
    } else if (action == "move_down") {
        emit moveDownPressed();
    } else if (action == "move_left") {
        emit moveLeftPressed();
    } else if (action == "move_right") {
        emit moveRightPressed();
    }
    // 战斗动作
    else if (action == "attack") {
        emit attackPressed();
    } else if (action == "defend") {
        emit defendPressed();
    }
    // 技能动作 - 解析技能索引
    else if (action.startsWith("skill_")) {
        bool ok;
        int skillIndex = action.mid(6).toInt(&ok);  // 提取"skill_"后的数字
        if (ok) {
            emit skillPressed(skillIndex);
        }
    }
    // 界面动作
    else if (action == "inventory") {
        emit inventoryPressed();
    } else if (action == "character") {
        emit characterPressed();
    } else if (action == "menu") {
        emit menuPressed();
    } else if (action == "map") {
        emit mapPressed();
    }
    // 系统动作
    else if (action == "pause") {
        emit pausePressed();
    } else if (action == "screenshot") {
        emit screenshotPressed();
    }
}
