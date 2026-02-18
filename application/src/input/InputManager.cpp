
#include "input/InputManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>

/*
 * 文件名: InputManager.cpp
 * 说明: 输入管理器实现
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 最后修改: 2025-08-18
 * 
 * 本文件实现了游戏的输入管理系统，负责处理键盘和鼠标输入，
 * 提供类型安全的键位绑定系统，并将输入转换为游戏动作信号。
 * 
 * 主要功能：
 * - 键盘事件处理和状态跟踪（基于Action枚举）
 * - 鼠标事件处理和状态跟踪
 * - 类型安全的按键/鼠标绑定系统
 * - 输入状态查询接口
 * - 游戏动作信号发送
 */

InputManager::InputManager(QObject *parent)
    : QObject(parent), m_inputFilterEnabled(false), m_updateTimer(nullptr)
{
    setupDefaultKeyMappings();
}

/**
 * @brief 析构函数
 * 
 * 释放资源
 */
InputManager::~InputManager() = default;

/**
 * @brief 处理键盘按下事件
 * @param event 键盘事件对象
 * @return 是否处理了该事件
 * 
 * 记录按键状态，查找并触发对应Action
 */
bool InputManager::handleKeyPress(QKeyEvent *event)
{
    if (!event) return false;
    Qt::Key key = static_cast<Qt::Key>(event->key());
    m_pressedKeys.insert(key);
    if (m_keyActionMap.contains(key)) {
        Action action = m_keyActionMap.value(key);
        m_pressedActions.insert(action);
        emit actionTriggered(action);
        emit actionPressed(action);
        return true;
    }
    return false;
}

/**
 * @brief 处理键盘释放事件
 * @param event 键盘事件对象
 * @return 是否处理了该事件
 * 
 * 移除按键状态，查找并释放对应Action
 */
bool InputManager::handleKeyRelease(QKeyEvent *event)
{
    if (!event) return false;
    Qt::Key key = static_cast<Qt::Key>(event->key());
    m_pressedKeys.remove(key);
    if (m_keyActionMap.contains(key)) {
        Action action = m_keyActionMap.value(key);
        m_pressedActions.remove(action);
        emit actionReleased(action);
        return true;
    }
    return false;
}

/**
 * @brief 处理鼠标按下事件
 * @param event 鼠标事件对象
 * @return 是否处理了该事件
 * 
 * 记录鼠标按钮状态，查找并触发对应Action
 */
bool InputManager::handleMousePress(QMouseEvent *event)
{
    if (!event) return false;
    MouseButton button = static_cast<MouseButton>(event->button());
    m_pressedMouseButtons.insert(button);
    if (m_mouseActionMap.contains(button)) {
        Action action = m_mouseActionMap.value(button);
        m_pressedActions.insert(action);
        emit actionTriggered(action);
        emit actionPressed(action);
        return true;
    }
    return false;
}

/**
 * @brief 处理鼠标释放事件
 * @param event 鼠标事件对象
 * @return 是否处理了该事件
 * 
 * 移除鼠标按钮状态，查找并释放对应Action
 */
bool InputManager::handleMouseRelease(QMouseEvent *event)
{
    if (!event) return false;
    MouseButton button = static_cast<MouseButton>(event->button());
    m_pressedMouseButtons.remove(button);
    if (m_mouseActionMap.contains(button)) {
        Action action = m_mouseActionMap.value(button);
        m_pressedActions.remove(action);
        emit actionReleased(action);
        return true;
    }
    return false;
}

/**
 * @brief 处理鼠标移动事件
 * @param event 鼠标事件对象
 * @return 是否处理了该事件
 * 
 * 更新鼠标位置和增量，发送mouseMoved信号
 */
bool InputManager::handleMouseMove(QMouseEvent *event)
{
    if (!event) return false;
    QPoint newPos = event->pos();
    m_mouseDelta = newPos - m_mousePosition;
    m_lastMousePosition = m_mousePosition;
    m_mousePosition = newPos;
    emit mouseMoved(m_mousePosition, m_mouseDelta);
    return true;
}

/**
 * @brief 处理鼠标滚轮事件
 * @param event 滚轮事件对象
 * @return 是否处理了该事件
 * 
 * 发送mouseWheelScrolled信号
 */
bool InputManager::handleWheelEvent(QWheelEvent *event)
{
    if (!event) return false;
    emit mouseWheelScrolled(event->angleDelta().y());
    return true;
}

/**
 * @brief 检查指定按键是否被按下
 * @param key 按键码
 * @return 是否被按下
 */
bool InputManager::isKeyPressed(Qt::Key key) const
{
    return m_pressedKeys.contains(key);
}

/**
 * @brief 检查指定Action是否被激活
 * @param action 游戏动作
 * @return 是否被激活
 */
bool InputManager::isActionPressed(Action action) const
{
    return m_pressedActions.contains(action);
}

/**
 * @brief 检查指定鼠标按钮是否被按下
 * @param button 鼠标按钮
 * @return 是否被按下
 */
bool InputManager::isMouseButtonPressed(MouseButton button) const
{
    return m_pressedMouseButtons.contains(button);
}

/**
 * @brief 获取当前鼠标位置
 * @return 鼠标屏幕坐标
 */
QPoint InputManager::getMousePosition() const
{
    return m_mousePosition;
}

/**
 * @brief 获取鼠标移动增量
 * @return 相对于上一帧的移动量
 */
QPoint InputManager::getMouseDelta() const
{
    return m_mouseDelta;
}

/**
 * @brief 将按键映射到游戏动作
 * @param key 按键码
 * @param action 游戏动作
 */
void InputManager::mapKeyToAction(Qt::Key key, Action action)
{
    m_keyActionMap[key] = action;
    m_actionKeyMap[action] = key;
}

/**
 * @brief 将鼠标按钮映射到游戏动作
 * @param button 鼠标按钮
 * @param action 游戏动作
 */
void InputManager::mapMouseButtonToAction(MouseButton button, Action action)
{
    m_mouseActionMap[button] = action;
    m_actionMouseMap[action] = button;
}

/**
 * @brief 取消按键映射
 * @param key 要取消映射的按键
 */
void InputManager::unmapKey(Qt::Key key)
{
    if (m_keyActionMap.contains(key)) {
        Action action = m_keyActionMap.value(key);
        m_keyActionMap.remove(key);
        m_actionKeyMap.remove(action);
    }
}

/**
 * @brief 取消鼠标按钮映射
 * @param button 要取消映射的鼠标按钮
 */
void InputManager::unmapMouseButton(MouseButton button)
{
    if (m_mouseActionMap.contains(button)) {
        Action action = m_mouseActionMap.value(button);
        m_mouseActionMap.remove(button);
        m_actionMouseMap.remove(action);
    }
}

/**
 * @brief 获取动作对应的按键
 * @param action 游戏动作
 * @return 映射的按键，未找到返回Qt::Key_unknown
 */
Qt::Key InputManager::getKeyForAction(Action action) const
{
    return m_actionKeyMap.value(action, Qt::Key_unknown);
}

/**
 * @brief 获取动作对应的鼠标按钮
 * @param action 游戏动作
 * @return 映射的鼠标按钮
 */
InputManager::MouseButton InputManager::getMouseButtonForAction(Action action) const
{
    return m_actionMouseMap.value(action, MouseButton::Left);
}

/**
 * @brief 清除所有输入映射
 */
void InputManager::clearAllMappings()
{
    m_keyActionMap.clear();
    m_actionKeyMap.clear();
    m_mouseActionMap.clear();
    m_actionMouseMap.clear();
}

/**
 * @brief 重置为默认按键映射
 */
void InputManager::resetToDefaultMappings()
{
    clearAllMappings();
    setupDefaultKeyMappings();
}

/**
 * @brief 检查组合键是否被按下
 * @param keys 按键组合列表
 * @return 所有按键是否同时被按下
 */
bool InputManager::isKeyCombinationPressed(const QList<Qt::Key>& keys) const
{
    for (const Qt::Key& key : keys) {
        if (!m_pressedKeys.contains(key)) return false;
    }
    return true;
}

/**
 * @brief 注册按键序列（简化实现）
 * @param sequence 按键序列
 * @param action 触发的动作
 * @param timeoutMs 超时时间
 */
void InputManager::registerKeySequence(const QList<Qt::Key>& sequence, Action action, int timeoutMs)
{
    KeySequence seq;
    seq.keys = sequence;
    seq.action = action;
    seq.timeoutMs = timeoutMs;
    seq.currentIndex = 0;
    seq.timer = new QTimer(this);
    seq.timer->setSingleShot(true);
    connect(seq.timer, &QTimer::timeout, this, &InputManager::onKeySequenceTimeout);
    m_keySequences.append(seq);
}

/**
 * @brief 取消按键序列注册（简化实现）
 * @param sequence 要取消的按键序列
 */
void InputManager::unregisterKeySequence(const QList<Qt::Key>& sequence)
{
    for (int i = m_keySequences.size() - 1; i >= 0; --i) {
        if (m_keySequences[i].keys == sequence) {
            delete m_keySequences[i].timer;
            m_keySequences.removeAt(i);
        }
    }
}

/**
 * @brief 设置输入过滤器
 * @param enabled 是否启用过滤
 */
void InputManager::setInputFilterEnabled(bool enabled)
{
    m_inputFilterEnabled = enabled;
}

/**
 * @brief 检查输入过滤器是否启用
 * @return 是否启用
 */
bool InputManager::isInputFilterEnabled() const
{
    return m_inputFilterEnabled;
}

/**
 * @brief 设置动作优先级
 * @param action 游戏动作
 * @param priority 优先级
 */
void InputManager::setActionPriority(Action action, int priority)
{
    m_actionPriorities[action] = priority;
}

/**
 * @brief 获取动作优先级
 * @param action 游戏动作
 * @return 优先级值
 */
int InputManager::getActionPriority(Action action) const
{
    return m_actionPriorities.value(action, 0);
}

/**
 * @brief 保存输入配置到文件（简化实现）
 * @param filePath 配置文件路径
 * @return 保存是否成功
 */
bool InputManager::saveInputConfig(const QString& filePath) const
{
    QString path = filePath.isEmpty() ? getDefaultConfigPath() : filePath;

    QJsonObject root;
    QJsonArray keyMappings;
    for (auto it = m_keyActionMap.constBegin(); it != m_keyActionMap.constEnd(); ++it) {
        QJsonObject mapping;
        mapping["key"] = static_cast<int>(it.key());
        mapping["action"] = static_cast<int>(it.value());
        keyMappings.append(mapping);
    }
    root["keyMappings"] = keyMappings;

    QJsonArray mouseMappings;
    for (auto it = m_mouseActionMap.constBegin(); it != m_mouseActionMap.constEnd(); ++it) {
        QJsonObject mapping;
        mapping["button"] = static_cast<int>(it.key());
        mapping["action"] = static_cast<int>(it.value());
        mouseMappings.append(mapping);
    }
    root["mouseMappings"] = mouseMappings;

    QJsonDocument doc(root);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "InputManager: 无法保存输入配置:" << path;
        return false;
    }
    file.write(doc.toJson());
    file.close();

    qDebug() << "InputManager: 输入配置已保存到:" << path;
    return true;
}

/**
 * @brief 从文件加载输入配置（简化实现）
 * @param filePath 配置文件路径
 * @return 加载是否成功
 */
bool InputManager::loadInputConfig(const QString& filePath)
{
    QString path = filePath.isEmpty() ? getDefaultConfigPath() : filePath;

    QFile file(path);
    if (!file.exists()) {
        qDebug() << "InputManager: 配置文件不存在，使用默认配置:" << path;
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "InputManager: 无法读取输入配置:" << path;
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "InputManager: 配置文件解析失败:" << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    clearAllMappings();

    // 加载键盘映射
    QJsonArray keyMappings = root["keyMappings"].toArray();
    for (const QJsonValue &val : keyMappings) {
        QJsonObject mapping = val.toObject();
        Qt::Key key = static_cast<Qt::Key>(mapping["key"].toInt());
        Action action = static_cast<Action>(mapping["action"].toInt());
        mapKeyToAction(key, action);
    }

    // 加载鼠标映射
    QJsonArray mouseMappings = root["mouseMappings"].toArray();
    for (const QJsonValue &val : mouseMappings) {
        QJsonObject mapping = val.toObject();
        MouseButton button = static_cast<MouseButton>(mapping["button"].toInt());
        Action action = static_cast<Action>(mapping["action"].toInt());
        mapMouseButtonToAction(button, action);
    }

    qDebug() << "InputManager: 输入配置已加载:" << path;
    return true;
}

/**
 * @brief 获取动作的显示名称
 * @param action 游戏动作
 * @return 本地化的动作名称
 */
QString InputManager::getActionDisplayName(Action action)
{
    static const QMap<Action, QString> actionNames = {
        {Action::MoveUp, "向上移动"},
        {Action::MoveDown, "向下移动"},
        {Action::MoveLeft, "向左移动"},
        {Action::MoveRight, "向右移动"},
        {Action::Attack, "普通攻击"},
        {Action::HeavyAttack, "重击"},
        {Action::Defend, "防御"},
        {Action::Dodge, "闪避"},
        {Action::Interact, "交互"},
        {Action::UseSkill1, "技能1"},
        {Action::UseSkill2, "技能2"},
        {Action::UseSkill3, "技能3"},
        {Action::UseSkill4, "技能4"},
        {Action::UseItem, "使用物品"},
        {Action::OpenInventory, "背包"},
        {Action::OpenCharacter, "角色"},
        {Action::OpenMap, "地图"},
        {Action::OpenMenu, "菜单"},
        {Action::OpenSkills, "技能树"},
        {Action::OpenQuests, "任务"},
        {Action::Pause, "暂停"},
        {Action::Confirm, "确认"},
        {Action::Screenshot, "截图"},
        {Action::QuickSave, "快速保存"},
        {Action::QuickLoad, "快速加载"},
        {Action::ToggleFullscreen, "全屏切换"},
    };
    return actionNames.value(action, QString("未知动作(%1)").arg(static_cast<int>(action)));
}

/**
 * @brief 获取按键的显示名称
 * @param key 按键码
 * @return 本地化的按键名称
 */
QString InputManager::getKeyDisplayName(Qt::Key key)
{
    return QKeySequence(key).toString();
}

/**
 * @brief 设置默认按键映射
 * 
 * 初始化默认的按键绑定配置
 */
void InputManager::setupDefaultKeyMappings()
{
    // 仅示例，实际可根据 Action 枚举完善
    mapKeyToAction(Qt::Key_W, Action::MoveUp);
    mapKeyToAction(Qt::Key_S, Action::MoveDown);
    mapKeyToAction(Qt::Key_A, Action::MoveLeft);
    mapKeyToAction(Qt::Key_D, Action::MoveRight);
    mapKeyToAction(Qt::Key_Space, Action::Attack);
    mapKeyToAction(Qt::Key_Shift, Action::Defend);
    mapKeyToAction(Qt::Key_1, Action::UseSkill1);
    mapKeyToAction(Qt::Key_2, Action::UseSkill2);
    mapKeyToAction(Qt::Key_3, Action::UseSkill3);
    mapKeyToAction(Qt::Key_4, Action::UseSkill4);
    mapKeyToAction(Qt::Key_I, Action::OpenInventory);
    mapKeyToAction(Qt::Key_C, Action::OpenCharacter);
    mapKeyToAction(Qt::Key_M, Action::OpenMap);
    mapKeyToAction(Qt::Key_Escape, Action::OpenMenu);
    mapKeyToAction(Qt::Key_P, Action::Pause);
    mapKeyToAction(Qt::Key_F12, Action::Screenshot);
    mapKeyToAction(Qt::Key_E, Action::Interact);
    mapKeyToAction(Qt::Key_Q, Action::Dodge);
    mapKeyToAction(Qt::Key_K, Action::OpenSkills);
    mapKeyToAction(Qt::Key_J, Action::OpenQuests);
    mapKeyToAction(Qt::Key_F, Action::UseItem);
    mapKeyToAction(Qt::Key_Return, Action::Confirm);
    mapKeyToAction(Qt::Key_F5, Action::QuickSave);
    mapKeyToAction(Qt::Key_F9, Action::QuickLoad);
    mapKeyToAction(Qt::Key_F11, Action::ToggleFullscreen);

    // 鼠标默认映射
    mapMouseButtonToAction(MouseButton::Left, Action::Attack);
    mapMouseButtonToAction(MouseButton::Right, Action::HeavyAttack);
}

void InputManager::handleActionTrigger(Action action, bool pressed)
{
    if (m_inputFilterEnabled && shouldFilterAction(action)) {
        return;
    }

    if (pressed) {
        m_pressedActions.insert(action);
        emit actionTriggered(action);
        emit actionPressed(action);
    } else {
        m_pressedActions.remove(action);
        emit actionReleased(action);
    }
}

void InputManager::updateKeySequences(Qt::Key key)
{
    for (auto &seq : m_keySequences) {
        if (seq.currentIndex < seq.keys.size() && seq.keys[seq.currentIndex] == key) {
            seq.currentIndex++;
            if (seq.currentIndex >= seq.keys.size()) {
                // 序列完成
                emit keySequenceTriggered(seq.keys, seq.action);
                emit actionTriggered(seq.action);
                seq.currentIndex = 0;
            } else if (seq.timer) {
                seq.timer->start(seq.timeoutMs);
            }
        } else {
            seq.currentIndex = 0;
        }
    }
}

bool InputManager::shouldFilterAction(Action action) const
{
    // 系统级动作不过滤
    if (action == Action::OpenMenu || action == Action::Pause ||
        action == Action::QuickSave || action == Action::Screenshot) {
        return false;
    }
    return m_inputFilterEnabled;
}

InputManager::Action InputManager::getHighestPriorityAction(const QList<Action> &actions) const
{
    if (actions.isEmpty()) {
        return Action::ActionCount;
    }

    Action best = actions.first();
    int bestPriority = m_actionPriorities.value(best, 0);

    for (const Action &a : actions) {
        int p = m_actionPriorities.value(a, 0);
        if (p > bestPriority) {
            bestPriority = p;
            best = a;
        }
    }

    return best;
}

bool InputManager::isValidKeyMapping(Qt::Key key, Action action) const
{
    Q_UNUSED(key)
    Q_UNUSED(action)
    // 所有映射默认合法
    return true;
}

QString InputManager::getDefaultConfigPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/input_config.json";
}

void InputManager::onKeySequenceTimeout()
{
    // 超时后重置所有序列匹配状态
    for (auto &seq : m_keySequences) {
        seq.currentIndex = 0;
    }
}

void InputManager::updateInputState()
{
    // 帧更新时重置鼠标增量
    m_mouseDelta = QPoint(0, 0);
}
