/*
 * 文件名: InputManager.h
 * 说明: 输入管理器头文件 - 统一管理键盘、鼠标和游戏手柄输入
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 统一处理键盘、鼠标输入事件
 * - 提供输入动作映射系统
 * - 支持自定义按键绑定
 * - 实现输入状态查询和缓存
 * - 支持输入组合键和序列检测
 * - 提供输入事件的优先级处理
 * 
 * 设计特点:
 * - 事件驱动：通过Qt信号槽分发输入事件
 * - 可配置：支持运行时修改按键映射
 * - 高性能：输入状态缓存，避免重复查询
 * - 扩展性：易于添加新的输入设备支持
 * 
 * 依赖项:
 * - Qt Core (QObject, QMap, QSet等)
 * - Qt GUI (QKeyEvent, QMouseEvent等)
 * 
 * 使用示例:
 * InputManager* input = InputManager::instance();
 * input->mapKeyToAction(Qt::Key_W, InputManager::Action::MoveUp);
 * if (input->isActionPressed(InputManager::Action::Attack)) {
 *     // 处理攻击动作
 * }
 */

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QPoint>
#include <QMutex>

/**
 * @brief 输入管理器
 * 
 * 这是游戏输入系统的核心类，负责处理所有用户输入并将其转换为游戏动作。
 * 它提供了灵活的输入映射系统，支持自定义按键绑定，并通过信号槽机制
 * 将输入事件分发给相应的游戏系统。
 * 
 * 主要功能：
 * 1. 输入事件处理（键盘、鼠标）
 * 2. 输入动作映射和管理
 * 3. 输入状态查询和缓存
 * 4. 自定义按键绑定支持
 * 5. 输入组合键检测
 * 6. 输入事件优先级处理
 */
class InputManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 游戏动作枚举
     * 
     * 定义游戏中所有可能的输入动作，这些动作与具体的按键解耦，
     * 允许用户自定义按键映射
     */
    enum class Action {
        // ==================== 移动动作 ====================
        MoveUp,             ///< 向上移动
        MoveDown,           ///< 向下移动
        MoveLeft,           ///< 向左移动
        MoveRight,          ///< 向右移动
        MoveUpLeft,         ///< 向左上移动
        MoveUpRight,        ///< 向右上移动
        MoveDownLeft,       ///< 向左下移动
        MoveDownRight,      ///< 向右下移动
        
        // ==================== 战斗动作 ====================
        Attack,             ///< 普通攻击
        HeavyAttack,        ///< 重攻击
        Defend,             ///< 防御/格挡
        Dodge,              ///< 闪避
        UseSkill1,          ///< 使用技能1
        UseSkill2,          ///< 使用技能2
        UseSkill3,          ///< 使用技能3
        UseSkill4,          ///< 使用技能4
        
        // ==================== 物品和界面 ====================
        UseItem,            ///< 使用物品
        OpenInventory,      ///< 打开物品栏
        OpenCharacter,      ///< 打开角色界面
        OpenSkills,         ///< 打开技能界面
        OpenQuests,         ///< 打开任务界面
        OpenMap,            ///< 打开地图
        
        // ==================== 系统动作 ====================
        OpenMenu,           ///< 打开主菜单
        Pause,              ///< 暂停游戏
        Confirm,            ///< 确认
        Cancel,             ///< 取消
        Interact,           ///< 交互
        
        // ==================== 相机控制 ====================
        CameraUp,           ///< 相机向上
        CameraDown,         ///< 相机向下
        CameraLeft,         ///< 相机向左
        CameraRight,        ///< 相机向右
        CameraZoomIn,       ///< 相机拉近
        CameraZoomOut,      ///< 相机拉远
        CameraReset,        ///< 重置相机
        
        // ==================== 快捷动作 ====================
        QuickSave,          ///< 快速保存
        QuickLoad,          ///< 快速加载
        Screenshot,         ///< 截图
        ToggleFullscreen,   ///< 切换全屏
        
        // 动作总数（用于数组大小等）
        ActionCount
    };
    Q_ENUM(Action)  // 使枚举在QML中可用
    
    /**
     * @brief 输入设备类型枚举
     */
    enum class InputDevice {
        Keyboard,           ///< 键盘
        Mouse,              ///< 鼠标
        Gamepad             ///< 游戏手柄（预留）
    };
    Q_ENUM(InputDevice)
    
    /**
     * @brief 鼠标按钮枚举
     */
    enum class MouseButton {
        Left = Qt::LeftButton,
        Right = Qt::RightButton,
        Middle = Qt::MiddleButton,
        Back = Qt::BackButton,
        Forward = Qt::ForwardButton
    };
    Q_ENUM(MouseButton)
    
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit InputManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~InputManager();
    
    // ==================== 输入事件处理 ====================
    
    /**
     * @brief 处理键盘按下事件
     * @param event 键盘事件对象
     * @return bool 事件是否被处理
     * 
     * 处理键盘按键按下，更新按键状态并触发相应动作
     */
    bool handleKeyPress(QKeyEvent *event);
    
    /**
     * @brief 处理键盘释放事件
     * @param event 键盘事件对象
     * @return bool 事件是否被处理
     */
    bool handleKeyRelease(QKeyEvent *event);
    
    /**
     * @brief 处理鼠标按下事件
     * @param event 鼠标事件对象
     * @return bool 事件是否被处理
     */
    bool handleMousePress(QMouseEvent *event);
    
    /**
     * @brief 处理鼠标释放事件
     * @param event 鼠标事件对象
     * @return bool 事件是否被处理
     */
    bool handleMouseRelease(QMouseEvent *event);
    
    /**
     * @brief 处理鼠标移动事件
     * @param event 鼠标事件对象
     * @return bool 事件是否被处理
     */
    bool handleMouseMove(QMouseEvent *event);
    
    /**
     * @brief 处理鼠标滚轮事件
     * @param event 滚轮事件对象
     * @return bool 事件是否被处理
     */
    bool handleWheelEvent(QWheelEvent *event);
    
    // ==================== 输入状态查询 ====================
    
    /**
     * @brief 检查指定按键是否被按下
     * @param key 按键码
     * @return bool 是否被按下
     */
    bool isKeyPressed(Qt::Key key) const;
    
    /**
     * @brief 检查指定动作是否被激活
     * @param action 游戏动作
     * @return bool 是否被激活
     */
    bool isActionPressed(Action action) const;
    
    /**
     * @brief 检查指定鼠标按钮是否被按下
     * @param button 鼠标按钮
     * @return bool 是否被按下
     */
    bool isMouseButtonPressed(MouseButton button) const;
    
    /**
     * @brief 获取当前鼠标位置
     * @return QPoint 鼠标屏幕坐标
     */
    QPoint getMousePosition() const;
    
    /**
     * @brief 获取鼠标移动增量
     * @return QPoint 相对于上一帧的移动量
     */
    QPoint getMouseDelta() const;
    
    // ==================== 输入映射管理 ====================
    
    /**
     * @brief 将按键映射到游戏动作
     * @param key 按键码
     * @param action 游戏动作
     * 
     * 建立按键与游戏动作的映射关系，支持一键多动作
     */
    void mapKeyToAction(Qt::Key key, Action action);
    
    /**
     * @brief 将鼠标按钮映射到游戏动作
     * @param button 鼠标按钮
     * @param action 游戏动作
     */
    void mapMouseButtonToAction(MouseButton button, Action action);
    
    /**
     * @brief 取消按键映射
     * @param key 要取消映射的按键
     */
    void unmapKey(Qt::Key key);
    
    /**
     * @brief 取消鼠标按钮映射
     * @param button 要取消映射的鼠标按钮
     */
    void unmapMouseButton(MouseButton button);
    
    /**
     * @brief 获取动作对应的按键
     * @param action 游戏动作
     * @return Qt::Key 映射的按键，未找到返回Qt::Key_unknown
     */
    Qt::Key getKeyForAction(Action action) const;
    
    /**
     * @brief 获取动作对应的鼠标按钮
     * @param action 游戏动作
     * @return MouseButton 映射的鼠标按钮
     */
    MouseButton getMouseButtonForAction(Action action) const;
    
    /**
     * @brief 清除所有输入映射
     */
    void clearAllMappings();
    
    /**
     * @brief 重置为默认按键映射
     */
    void resetToDefaultMappings();
    
    // ==================== 组合键和序列检测 ====================
    
    /**
     * @brief 检查组合键是否被按下
     * @param keys 按键组合列表
     * @return bool 所有按键是否同时被按下
     */
    bool isKeyCombinationPressed(const QList<Qt::Key>& keys) const;
    
    /**
     * @brief 注册按键序列
     * @param sequence 按键序列
     * @param action 触发的动作
     * @param timeoutMs 序列超时时间（毫秒）
     */
    void registerKeySequence(const QList<Qt::Key>& sequence, Action action, int timeoutMs = 1000);
    
    /**
     * @brief 取消按键序列注册
     * @param sequence 要取消的按键序列
     */
    void unregisterKeySequence(const QList<Qt::Key>& sequence);
    
    // ==================== 输入过滤和优先级 ====================
    
    /**
     * @brief 设置输入过滤器
     * @param enabled 是否启用过滤
     * 
     * 启用后，某些输入可能被过滤掉（如UI界面打开时过滤游戏输入）
     */
    void setInputFilterEnabled(bool enabled);
    
    /**
     * @brief 检查输入过滤器是否启用
     * @return bool 是否启用
     */
    bool isInputFilterEnabled() const;
    
    /**
     * @brief 设置动作优先级
     * @param action 游戏动作
     * @param priority 优先级（数值越大优先级越高）
     */
    void setActionPriority(Action action, int priority);
    
    /**
     * @brief 获取动作优先级
     * @param action 游戏动作
     * @return int 优先级值
     */
    int getActionPriority(Action action) const;
    
    // ==================== 配置管理 ====================
    
    /**
     * @brief 保存输入配置到文件
     * @param filePath 配置文件路径，为空则使用默认路径
     * @return bool 保存是否成功
     */
    bool saveInputConfig(const QString& filePath = QString()) const;
    
    /**
     * @brief 从文件加载输入配置
     * @param filePath 配置文件路径，为空则使用默认路径
     * @return bool 加载是否成功
     */
    bool loadInputConfig(const QString& filePath = QString());
    
    /**
     * @brief 获取动作的显示名称
     * @param action 游戏动作
     * @return QString 本地化的动作名称
     */
    static QString getActionDisplayName(Action action);
    
    /**
     * @brief 获取按键的显示名称
     * @param key 按键码
     * @return QString 本地化的按键名称
     */
    static QString getKeyDisplayName(Qt::Key key);

signals:
    // ==================== 动作信号 ====================
    
    /**
     * @brief 动作触发信号（按下时触发一次）
     * @param action 触发的动作
     */
    void actionTriggered(Action action);
    
    /**
     * @brief 动作按下信号（持续按下时持续触发）
     * @param action 按下的动作
     */
    void actionPressed(Action action);
    
    /**
     * @brief 动作释放信号
     * @param action 释放的动作
     */
    void actionReleased(Action action);
    
    // ==================== 鼠标信号 ====================
    
    /**
     * @brief 鼠标移动信号
     * @param position 新的鼠标位置
     * @param delta 移动增量
     */
    void mouseMoved(const QPoint& position, const QPoint& delta);
    
    /**
     * @brief 鼠标滚轮信号
     * @param delta 滚轮增量
     */
    void mouseWheelScrolled(int delta);
    
    // ==================== 配置信号 ====================
    
    /**
     * @brief 输入映射变更信号
     * @param action 变更的动作
     * @param oldKey 旧按键
     * @param newKey 新按键
     */
    void inputMappingChanged(Action action, Qt::Key oldKey, Qt::Key newKey);
    
    /**
     * @brief 按键序列触发信号
     * @param sequence 触发的按键序列
     * @param action 对应的动作
     */
    void keySequenceTriggered(const QList<Qt::Key>& sequence, Action action);

private slots:
    /**
     * @brief 按键序列超时处理
     */
    void onKeySequenceTimeout();
    
    /**
     * @brief 输入状态更新定时器
     */
    void updateInputState();

private:
    // ==================== 私有成员变量 ====================
    
    // 输入状态缓存
    QSet<Qt::Key> m_pressedKeys;                    ///< 当前按下的按键集合
    QSet<MouseButton> m_pressedMouseButtons;       ///< 当前按下的鼠标按钮集合
    QSet<Action> m_pressedActions;                  ///< 当前激活的动作集合
    
    // 鼠标状态
    QPoint m_mousePosition;                         ///< 当前鼠标位置
    QPoint m_lastMousePosition;                     ///< 上一帧鼠标位置
    QPoint m_mouseDelta;                            ///< 鼠标移动增量
    
    // 输入映射
    QMap<Qt::Key, Action> m_keyActionMap;           ///< 按键到动作的映射
    QMap<MouseButton, Action> m_mouseActionMap;     ///< 鼠标按钮到动作的映射
    QMap<Action, Qt::Key> m_actionKeyMap;           ///< 动作到按键的反向映射
    QMap<Action, MouseButton> m_actionMouseMap;     ///< 动作到鼠标按钮的反向映射
    
    // 组合键和序列
    struct KeySequence {
        QList<Qt::Key> keys;                        ///< 按键序列
        Action action;                              ///< 对应动作
        int timeoutMs;                              ///< 超时时间
        QTimer* timer;                              ///< 超时定时器
        int currentIndex;                           ///< 当前匹配索引
    };
    QList<KeySequence> m_keySequences;             ///< 注册的按键序列
    
    // 优先级和过滤
    QMap<Action, int> m_actionPriorities;          ///< 动作优先级映射
    bool m_inputFilterEnabled;                      ///< 输入过滤器状态
    
    // 状态更新
    QTimer* m_updateTimer;                          ///< 输入状态更新定时器
    
    // 线程安全
    mutable QMutex m_mutex;                         ///< 互斥锁
    
    // ==================== 私有方法 ====================
    
    /**
     * @brief 设置默认按键映射
     * 
     * 初始化默认的按键绑定配置
     */
    void setupDefaultKeyMappings();
    
    /**
     * @brief 处理动作触发
     * @param action 触发的动作
     * @param pressed 是否为按下事件
     */
    void handleActionTrigger(Action action, bool pressed);
    
    /**
     * @brief 更新按键序列状态
     * @param key 按下的按键
     */
    void updateKeySequences(Qt::Key key);
    
    /**
     * @brief 检查动作是否应该被过滤
     * @param action 要检查的动作
     * @return bool 是否应该过滤
     */
    bool shouldFilterAction(Action action) const;
    
    /**
     * @brief 获取有效的动作（考虑优先级）
     * @param actions 候选动作列表
     * @return Action 最高优先级的动作
     */
    Action getHighestPriorityAction(const QList<Action>& actions) const;
    
    /**
     * @brief 验证按键映射的合法性
     * @param key 按键
     * @param action 动作
     * @return bool 映射是否合法
     */
    bool isValidKeyMapping(Qt::Key key, Action action) const;
    
    /**
     * @brief 获取默认配置文件路径
     * @return QString 配置文件路径
     */
    QString getDefaultConfigPath() const;
};

#endif // INPUTMANAGER_H
