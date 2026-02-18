/*
 * 文件名: GameplayWindow.cpp
 * 说明: 游戏玩法窗口实现文件
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 游戏主要玩法界面的实现
 * - 集成游戏世界渲染和UI元素
 * - 处理玩家输入和游戏交互
 * - 显示游戏状态和信息面板
 */
#include "ui/GameplayWindow.h"
#include "game/InventorySystem.h"
#include "config/ConfigManager.h"
#include "utils/PerformanceMonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QTimer>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QDebug>

GameplayWindow::GameplayWindow(QWidget *parent)
    : QWidget(parent)
    , m_configManager(nullptr)
    , m_inventorySystem(nullptr)
    , m_performanceMonitor(nullptr)
    , m_gameView(nullptr)
    , m_playerInfoPanel(nullptr)
    , m_inventoryPanel(nullptr)
    , m_chatPanel(nullptr)
    , m_miniMapPanel(nullptr)
    , m_skillPanel(nullptr)
    , m_healthBar(nullptr)
    , m_manaBar(nullptr)
    , m_experienceBar(nullptr)
    , m_playerNameLabel(nullptr)
    , m_playerLevelLabel(nullptr)
    , m_chatTextEdit(nullptr)
    , m_logTextEdit(nullptr)
    , m_updateTimer(nullptr)
    , m_isInventoryVisible(true)
    , m_isChatVisible(true)
    , m_isSkillPanelVisible(false)
{
    qDebug() << "GameplayWindow: 开始初始化游戏玩法窗口";
    
    // 初始化组件
    initializeComponents();
    
    // 设置UI
    setupUI();
    
    // 建立连接
    setupConnections();
    
    // 应用样式
    applyStyles();
    
    // 启动更新定时器
    startUpdateTimer();
    
    qDebug() << "GameplayWindow: 游戏玩法窗口初始化完成";
}

GameplayWindow::~GameplayWindow()
{
    // 停止更新定时器
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    qDebug() << "GameplayWindow: 游戏玩法窗口销毁";
}

void GameplayWindow::keyPressEvent(QKeyEvent *event)
{
    // 处理游戏快捷键
    switch (event->key()) {
        case Qt::Key_I:
            toggleInventory();
            break;
        case Qt::Key_C:
            toggleChat();
            break;
        case Qt::Key_K:
            toggleSkillPanel();
            break;
        case Qt::Key_M:
            toggleMiniMap();
            break;
        case Qt::Key_Escape:
            showGameMenu();
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (m_isChatVisible) {
                focusChat();
            }
            break;
        default:
            // 将其他按键传递给游戏引擎处理
            QWidget::keyPressEvent(event);
            break;
    }
}

void GameplayWindow::mousePressEvent(QMouseEvent *event)
{
    // 处理鼠标点击事件
    if (event->button() == Qt::LeftButton) {
        // 左键点击处理
        handleLeftClick(event->pos());
    } else if (event->button() == Qt::RightButton) {
        // 右键点击处理
        handleRightClick(event->pos());
    }
    
    QWidget::mousePressEvent(event);
}

void GameplayWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    // 窗口显示时开始更新
    if (m_updateTimer) {
        m_updateTimer->start();
    }
}

void GameplayWindow::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    
    // 窗口隐藏时停止更新以节省资源
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

void GameplayWindow::toggleInventory()
{
    m_isInventoryVisible = !m_isInventoryVisible;
    
    if (m_inventoryPanel) {
        m_inventoryPanel->setVisible(m_isInventoryVisible);
    }
    
    qDebug() << "GameplayWindow: 背包面板" << (m_isInventoryVisible ? "显示" : "隐藏");
    emit inventoryToggled(m_isInventoryVisible);
}

void GameplayWindow::toggleChat()
{
    m_isChatVisible = !m_isChatVisible;
    
    if (m_chatPanel) {
        m_chatPanel->setVisible(m_isChatVisible);
    }
    
    qDebug() << "GameplayWindow: 聊天面板" << (m_isChatVisible ? "显示" : "隐藏");
    emit chatToggled(m_isChatVisible);
}

void GameplayWindow::toggleSkillPanel()
{
    m_isSkillPanelVisible = !m_isSkillPanelVisible;
    
    if (m_skillPanel) {
        m_skillPanel->setVisible(m_isSkillPanelVisible);
    }
    
    qDebug() << "GameplayWindow: 技能面板" << (m_isSkillPanelVisible ? "显示" : "隐藏");
    emit skillPanelToggled(m_isSkillPanelVisible);
}

void GameplayWindow::toggleMiniMap()
{
    bool visible = m_miniMapPanel ? m_miniMapPanel->isVisible() : false;
    visible = !visible;
    
    if (m_miniMapPanel) {
        m_miniMapPanel->setVisible(visible);
    }
    
    qDebug() << "GameplayWindow: 小地图" << (visible ? "显示" : "隐藏");
    emit miniMapToggled(visible);
}

void GameplayWindow::showGameMenu()
{
    qDebug() << "GameplayWindow: 显示游戏菜单";
    emit gameMenuRequested();
}

void GameplayWindow::focusChat()
{
    if (m_chatTextEdit && m_isChatVisible) {
        m_chatTextEdit->setFocus();
        qDebug() << "GameplayWindow: 聊天输入框获得焦点";
    }
}

void GameplayWindow::updatePlayerInfo(const QString &name, int level, int health, int maxHealth, int mana, int maxMana, int experience, int maxExperience)
{
    // 参数校验
    maxHealth = qMax(1, maxHealth);
    maxMana = qMax(1, maxMana);
    maxExperience = qMax(1, maxExperience);
    health = qBound(0, health, maxHealth);
    mana = qBound(0, mana, maxMana);
    experience = qBound(0, experience, maxExperience);

    // 更新玩家姓名和等级
    if (m_playerNameLabel) {
        m_playerNameLabel->setText(name);
    }

    if (m_playerLevelLabel) {
        m_playerLevelLabel->setText(QString("等级 %1").arg(level));
    }

    // 更新生命值
    if (m_healthBar) {
        m_healthBar->setMaximum(maxHealth);
        m_healthBar->setValue(health);
        m_healthBar->setFormat(QString("%1/%2").arg(health).arg(maxHealth));
    }

    // 更新魔法值
    if (m_manaBar) {
        m_manaBar->setMaximum(maxMana);
        m_manaBar->setValue(mana);
        m_manaBar->setFormat(QString("%1/%2").arg(mana).arg(maxMana));
    }

    // 更新经验值
    if (m_experienceBar) {
        m_experienceBar->setMaximum(maxExperience);
        m_experienceBar->setValue(experience);
        m_experienceBar->setFormat(QString("%1/%2").arg(experience).arg(maxExperience));
    }
}

void GameplayWindow::addChatMessage(const QString &sender, const QString &message, ChatMessageType type)
{
    if (!m_logTextEdit) {
        return;
    }

    // 限制消息数量，防止内存持续增长
    if (m_logTextEdit->document()->blockCount() > 200) {
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 50);
        cursor.removeSelectedText();
    }
    
    // 根据消息类型设置颜色
    QString color;
    switch (type) {
        case ChatMessageType::System:
            color = "#FFD700"; // 金色
            break;
        case ChatMessageType::Player:
            color = "#87CEEB"; // 天蓝色
            break;
        case ChatMessageType::NPC:
            color = "#98FB98"; // 淡绿色
            break;
        case ChatMessageType::Combat:
            color = "#FF6347"; // 番茄红
            break;
        default:
            color = "#FFFFFF"; // 白色
            break;
    }
    
    // 格式化消息
    QString formattedMessage = QString(
        "<span style='color: %1;'>[%2] %3: %4</span><br>"
    ).arg(color)
     .arg(QTime::currentTime().toString("hh:mm:ss"))
     .arg(sender)
     .arg(message);
    
    // 添加到聊天记录
    m_logTextEdit->append(formattedMessage);
    
    // 自动滚动到底部
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
    
    qDebug() << "GameplayWindow: 添加聊天消息 -" << sender << ":" << message;
}

void GameplayWindow::initializeComponents()
{
    // 获取配置管理器实例
    m_configManager = ConfigManager::instance();
    
    // 获取背包系统实例
    m_inventorySystem = new Game::InventorySystem(this);
    
    // 获取性能监控器实例
    m_performanceMonitor = PerformanceMonitor::instance();
    
    // 创建更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(50); // 20FPS更新频率
    
    qDebug() << "GameplayWindow: 组件初始化完成";
}

void GameplayWindow::setupUI()
{
    // 设置窗口属性
    setMinimumSize(1024, 768);
    
    // 创建主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // 创建左侧面板
    setupLeftPanel(mainLayout);
    
    // 创建中央游戏区域
    setupCentralArea(mainLayout);
    
    // 创建右侧面板
    setupRightPanel(mainLayout);
    
    qDebug() << "GameplayWindow: UI设置完成";
}

void GameplayWindow::setupLeftPanel(QHBoxLayout* mainLayout)
{
    // 创建左侧面板
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(250);
    leftPanel->setStyleSheet("background-color: rgba(30, 30, 30, 200); border-radius: 5px;");
    
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(10);
    
    // 玩家信息面板
    setupPlayerInfoPanel(leftLayout);
    
    // 技能面板
    setupSkillPanel(leftLayout);
    
    // 添加弹簧
    leftLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    mainLayout->addWidget(leftPanel);
}

void GameplayWindow::setupCentralArea(QHBoxLayout* mainLayout)
{
    // 创建中央区域
    QWidget* centralArea = new QWidget();
    
    QVBoxLayout* centralLayout = new QVBoxLayout(centralArea);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(5);
    
    // 游戏视图
    m_gameView = new QWidget();
    m_gameView->setStyleSheet("background-color: black; border: 2px solid #444;");
    m_gameView->setMinimumSize(640, 480);
    centralLayout->addWidget(m_gameView, 1);
    
    // 聊天面板
    setupChatPanel(centralLayout);
    
    mainLayout->addWidget(centralArea, 1);
}

void GameplayWindow::setupRightPanel(QHBoxLayout* mainLayout)
{
    // 创建右侧面板
    QWidget* rightPanel = new QWidget();
    rightPanel->setFixedWidth(300);
    rightPanel->setStyleSheet("background-color: rgba(30, 30, 30, 200); border-radius: 5px;");
    
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(10);
    
    // 小地图面板
    setupMiniMapPanel(rightLayout);
    
    // 背包面板
    setupInventoryPanel(rightLayout);
    
    mainLayout->addWidget(rightPanel);
}

void GameplayWindow::setupPlayerInfoPanel(QVBoxLayout* layout)
{
    // 创建玩家信息面板
    m_playerInfoPanel = new QGroupBox("玩家信息");
    m_playerInfoPanel->setFixedHeight(150);
    
    QVBoxLayout* playerLayout = new QVBoxLayout(m_playerInfoPanel);
    playerLayout->setContentsMargins(10, 15, 10, 10);
    playerLayout->setSpacing(8);
    
    // 玩家姓名和等级
    m_playerNameLabel = new QLabel("未知玩家");
    m_playerNameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #FFD700;");
    m_playerNameLabel->setAlignment(Qt::AlignCenter);
    playerLayout->addWidget(m_playerNameLabel);
    
    m_playerLevelLabel = new QLabel("等级 1");
    m_playerLevelLabel->setStyleSheet("font-size: 12px; color: #87CEEB;");
    m_playerLevelLabel->setAlignment(Qt::AlignCenter);
    playerLayout->addWidget(m_playerLevelLabel);
    
    // 生命值条
    QLabel* healthLabel = new QLabel("生命值:");
    healthLabel->setStyleSheet("font-size: 11px; color: #FFFFFF;");
    playerLayout->addWidget(healthLabel);
    
    m_healthBar = new QProgressBar();
    m_healthBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid #555;"
        "    border-radius: 3px;"
        "    text-align: center;"
        "    font-size: 10px;"
        "    color: white;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #FF4444;"
        "    border-radius: 2px;"
        "}"
    );
    m_healthBar->setMaximum(100);
    m_healthBar->setValue(100);
    m_healthBar->setFormat("%v/%m");
    playerLayout->addWidget(m_healthBar);
    
    // 魔法值条
    QLabel* manaLabel = new QLabel("魔法值:");
    manaLabel->setStyleSheet("font-size: 11px; color: #FFFFFF;");
    playerLayout->addWidget(manaLabel);
    
    m_manaBar = new QProgressBar();
    m_manaBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid #555;"
        "    border-radius: 3px;"
        "    text-align: center;"
        "    font-size: 10px;"
        "    color: white;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #4444FF;"
        "    border-radius: 2px;"
        "}"
    );
    m_manaBar->setMaximum(50);
    m_manaBar->setValue(50);
    m_manaBar->setFormat("%v/%m");
    playerLayout->addWidget(m_manaBar);
    
    // 经验值条
    QLabel* expLabel = new QLabel("经验值:");
    expLabel->setStyleSheet("font-size: 11px; color: #FFFFFF;");
    playerLayout->addWidget(expLabel);
    
    m_experienceBar = new QProgressBar();
    m_experienceBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid #555;"
        "    border-radius: 3px;"
        "    text-align: center;"
        "    font-size: 10px;"
        "    color: white;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #44FF44;"
        "    border-radius: 2px;"
        "}"
    );
    m_experienceBar->setMaximum(1000);
    m_experienceBar->setValue(0);
    m_experienceBar->setFormat("%v/%m");
    playerLayout->addWidget(m_experienceBar);
    
    layout->addWidget(m_playerInfoPanel);
}

void GameplayWindow::setupSkillPanel(QVBoxLayout* layout)
{
    // 创建技能面板
    m_skillPanel = new QGroupBox("技能");
    m_skillPanel->setFixedHeight(200);
    m_skillPanel->setVisible(m_isSkillPanelVisible);
    
    QGridLayout* skillLayout = new QGridLayout(m_skillPanel);
    skillLayout->setContentsMargins(10, 15, 10, 10);
    skillLayout->setSpacing(5);
    
    // 创建技能按钮
    for (int i = 0; i < 12; ++i) {
        QPushButton* skillButton = new QPushButton();
        skillButton->setFixedSize(40, 40);
        skillButton->setStyleSheet(
            "QPushButton {"
            "    background-color: rgba(60, 60, 60, 150);"
            "    border: 1px solid #666;"
            "    border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "    background-color: rgba(80, 80, 80, 200);"
            "    border: 1px solid #888;"
            "}"
            "QPushButton:pressed {"
            "    background-color: rgba(40, 40, 40, 200);"
            "}"
        );
        
        int row = i / 4;
        int col = i % 4;
        skillLayout->addWidget(skillButton, row, col);
        
        // 连接技能按钮点击事件
        connect(skillButton, &QPushButton::clicked, this, [this, i]() {
            onSkillButtonClicked(i);
        });
    }
    
    layout->addWidget(m_skillPanel);
}

void GameplayWindow::setupMiniMapPanel(QVBoxLayout* layout)
{
    // 创建小地图面板
    m_miniMapPanel = new QGroupBox("小地图");
    m_miniMapPanel->setFixedHeight(200);
    
    QVBoxLayout* mapLayout = new QVBoxLayout(m_miniMapPanel);
    mapLayout->setContentsMargins(10, 15, 10, 10);
    
    // 小地图显示区域
    QWidget* mapWidget = new QWidget();
    mapWidget->setStyleSheet("background-color: #222; border: 1px solid #555;");
    mapWidget->setMinimumSize(260, 160);
    mapLayout->addWidget(mapWidget);
    
    layout->addWidget(m_miniMapPanel);
}

void GameplayWindow::setupInventoryPanel(QVBoxLayout* layout)
{
    // 创建背包面板
    m_inventoryPanel = new QGroupBox("背包");
    m_inventoryPanel->setVisible(m_isInventoryVisible);
    
    QVBoxLayout* invLayout = new QVBoxLayout(m_inventoryPanel);
    invLayout->setContentsMargins(10, 15, 10, 10);
    invLayout->setSpacing(5);
    
    // 背包网格
    QWidget* inventoryGrid = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(inventoryGrid);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(2);
    
    // 创建背包槽位
    for (int i = 0; i < 25; ++i) { // 5x5网格
        QPushButton* slotButton = new QPushButton();
        slotButton->setFixedSize(45, 45);
        slotButton->setStyleSheet(
            "QPushButton {"
            "    background-color: rgba(60, 60, 60, 150);"
            "    border: 1px solid #666;"
            "    border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "    background-color: rgba(80, 80, 80, 200);"
            "    border: 1px solid #888;"
            "}"
        );
        
        int row = i / 5;
        int col = i % 5;
        gridLayout->addWidget(slotButton, row, col);
        
        // 连接槽位点击事件
        connect(slotButton, &QPushButton::clicked, this, [this, i]() {
            onInventorySlotClicked(i);
        });
    }
    
    invLayout->addWidget(inventoryGrid);
    
    // 背包操作按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* sortButton = new QPushButton("整理");
    sortButton->setFixedSize(60, 25);
    connect(sortButton, &QPushButton::clicked, this, &GameplayWindow::onSortInventoryClicked);
    buttonLayout->addWidget(sortButton);
    
    buttonLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    invLayout->addLayout(buttonLayout);
    
    layout->addWidget(m_inventoryPanel);
}

void GameplayWindow::setupChatPanel(QVBoxLayout* layout)
{
    // 创建聊天面板
    m_chatPanel = new QGroupBox("聊天");
    m_chatPanel->setFixedHeight(150);
    m_chatPanel->setVisible(m_isChatVisible);
    
    QVBoxLayout* chatLayout = new QVBoxLayout(m_chatPanel);
    chatLayout->setContentsMargins(10, 15, 10, 10);
    chatLayout->setSpacing(5);
    
    // 聊天记录显示
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(80);
    m_logTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: rgba(20, 20, 20, 200);"
        "    border: 1px solid #555;"
        "    border-radius: 3px;"
        "    color: white;"
        "    font-size: 11px;"
        "}"
    );
    chatLayout->addWidget(m_logTextEdit);
    
    // 聊天输入框
    QHBoxLayout* inputLayout = new QHBoxLayout();
    
    m_chatTextEdit = new QLineEdit();
    m_chatTextEdit->setPlaceholderText("输入聊天内容...");
    m_chatTextEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: rgba(40, 40, 40, 200);"
        "    border: 1px solid #666;"
        "    border-radius: 3px;"
        "    color: white;"
        "    padding: 5px;"
        "    font-size: 11px;"
        "}"
    );
    inputLayout->addWidget(m_chatTextEdit);
    
    QPushButton* sendButton = new QPushButton("发送");
    sendButton->setFixedSize(50, 25);
    connect(sendButton, &QPushButton::clicked, this, &GameplayWindow::onSendChatMessage);
    inputLayout->addWidget(sendButton);
    
    chatLayout->addLayout(inputLayout);
    
    layout->addWidget(m_chatPanel);
}

void GameplayWindow::setupConnections()
{
    // 连接更新定时器
    connect(m_updateTimer, &QTimer::timeout, this, &GameplayWindow::updateUI);
    
    // 连接聊天输入框回车事件
    connect(m_chatTextEdit, &QLineEdit::returnPressed, this, &GameplayWindow::onSendChatMessage);
    
    // 连接背包系统信号
    if (m_inventorySystem) {
        connect(m_inventorySystem, &Game::InventorySystem::itemAdded,
                this, &GameplayWindow::onItemAdded);
        connect(m_inventorySystem, &Game::InventorySystem::itemRemoved,
                this, &GameplayWindow::onItemRemoved);
        connect(m_inventorySystem, &Game::InventorySystem::inventoryFull,
                this, &GameplayWindow::onInventoryFull);
    }
    
    qDebug() << "GameplayWindow: 信号连接完成";
}

void GameplayWindow::applyStyles()
{
    // 设置窗口样式
    setStyleSheet(
        "GameplayWindow {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 rgba(20, 20, 40, 255), "
        "        stop:1 rgba(40, 40, 80, 255));"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #555;"
        "    border-radius: 5px;"
        "    margin-top: 10px;"
        "    color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
    );
    
    qDebug() << "GameplayWindow: 样式应用完成";
}

void GameplayWindow::startUpdateTimer()
{
    if (m_updateTimer) {
        m_updateTimer->start();
        qDebug() << "GameplayWindow: 更新定时器启动";
    }
}

void GameplayWindow::updateUI()
{
    // 更新游戏界面渲染
    if (m_gameView) {
        m_gameView->update();
    }

    // 更新性能信息
    if (m_performanceMonitor && m_performanceMonitor->isEnabled()) {
        // 可以在这里更新性能相关的UI元素
    }
}

void GameplayWindow::handleLeftClick(const QPoint &position)
{
    // 发射信号供外部处理（移动角色、选择目标等）
    emit leftClicked(position);
}

void GameplayWindow::handleRightClick(const QPoint &position)
{
    // 处理右键点击
    qDebug() << "GameplayWindow: 右键点击位置:" << position;

    // 发射信号
    emit rightClicked(position);

    // 弹出上下文菜单
    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu { background-color: #2c2c2c; color: white; border: 1px solid #555; }"
        "QMenu::item:selected { background-color: #3498db; }"
    );
    contextMenu.addAction("攻击", this, [this]() {
        addChatMessage("系统", "选择攻击目标", ChatMessageType::Combat);
    });
    contextMenu.addAction("交互", this, [this]() {
        addChatMessage("系统", "与目标交互", ChatMessageType::System);
    });
    contextMenu.addAction("查看", this, [this]() {
        addChatMessage("系统", "查看目标信息", ChatMessageType::System);
    });
    contextMenu.exec(mapToGlobal(position));
}

void GameplayWindow::onSkillButtonClicked(int skillIndex)
{
    qDebug() << "GameplayWindow: 技能按钮点击 - 索引:" << skillIndex;

    // 通过聊天消息反馈技能使用
    addChatMessage("系统", QString("释放技能 [%1]").arg(skillIndex + 1), ChatMessageType::Combat);
    emit skillUsed(skillIndex);
}

void GameplayWindow::onInventorySlotClicked(int slotIndex)
{
    qDebug() << "GameplayWindow: 背包槽位点击 - 索引:" << slotIndex;

    if (m_inventorySystem) {
        QJsonObject slotInfo = m_inventorySystem->getSlotInfo(slotIndex);
        if (!slotInfo.isEmpty()) {
            QString itemName = slotInfo["name"].toString();
            addChatMessage("系统", QString("选中物品: %1").arg(itemName), ChatMessageType::System);
            emit inventorySlotClicked(slotIndex, slotInfo);
        } else {
            addChatMessage("系统", QString("空槽位 [%1]").arg(slotIndex + 1), ChatMessageType::System);
        }
    }
}

void GameplayWindow::onSortInventoryClicked()
{
    qDebug() << "GameplayWindow: 整理背包按钮点击";
    
    if (m_inventorySystem) {
        m_inventorySystem->sortInventory();
        addChatMessage("系统", "背包已整理", ChatMessageType::System);
    }
}

void GameplayWindow::onSendChatMessage()
{
    if (!m_chatTextEdit || m_chatTextEdit->text().isEmpty()) {
        return;
    }
    
    QString message = m_chatTextEdit->text().trimmed();
    m_chatTextEdit->clear();
    
    // 添加到聊天记录
    addChatMessage("玩家", message, ChatMessageType::Player);
    
    // 发送聊天消息信号
    emit chatMessageSent(message);
    
    qDebug() << "GameplayWindow: 发送聊天消息:" << message;
}

void GameplayWindow::onItemAdded(int itemId, int quantity)
{
    addChatMessage("系统", 
        QString("获得物品: %1 x%2").arg(itemId).arg(quantity), 
        ChatMessageType::System);
}

void GameplayWindow::onItemRemoved(int itemId, int quantity)
{
    addChatMessage("系统", 
        QString("失去物品: %1 x%2").arg(itemId).arg(quantity), 
        ChatMessageType::System);
}

void GameplayWindow::onInventoryFull()
{
    addChatMessage("系统", "背包已满！", ChatMessageType::System);
}
