/*
 * 文件名: InventoryWindow.cpp
 * 说明: 背包窗口实现文件
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 背包界面的显示和管理
 * - 物品的拖拽和操作
 * - 背包容量和状态显示
 * - 物品详情和工具提示
 */

#include "ui/InventoryWindow.h"
#include "game/InventorySystem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include <QDebug>

InventoryWindow::InventoryWindow(QWidget *parent)
    : QWidget(parent)
    , m_inventorySystem(nullptr)
    , m_inventoryGrid(nullptr)
    , m_itemDetailPanel(nullptr)
    , m_capacityLabel(nullptr)
    , m_selectedSlot(-1)
{
    qDebug() << "InventoryWindow: 初始化背包窗口";
    
    // 初始化背包系统
    m_inventorySystem = new Game::InventorySystem(this);
    
    // 设置窗口属性
    setWindowTitle("背包");
    setMinimumSize(600, 400);
    resize(800, 600);
    
    // 设置UI
    setupUI();
    
    // 建立连接
    setupConnections();
    
    // 更新显示
    updateInventoryDisplay();
    
    qDebug() << "InventoryWindow: 背包窗口初始化完成";
}

InventoryWindow::~InventoryWindow()
{
    qDebug() << "InventoryWindow: 背包窗口销毁";
}

void InventoryWindow::setupUI()
{
    // 创建主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 创建分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧背包区域
    setupInventoryArea(splitter);
    
    // 右侧详情区域
    setupDetailArea(splitter);
    
    // 设置分割器比例
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
}

void InventoryWindow::setupInventoryArea(QSplitter* splitter)
{
    QWidget* inventoryWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(inventoryWidget);
    
    // 标题和容量信息
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* titleLabel = new QLabel("背包");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    m_capacityLabel = new QLabel("0/50");
    m_capacityLabel->setStyleSheet("color: #666;");
    headerLayout->addWidget(m_capacityLabel);
    
    layout->addLayout(headerLayout);
    
    // 背包网格
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget* gridWidget = new QWidget();
    m_inventoryGrid = new QGridLayout(gridWidget);
    m_inventoryGrid->setSpacing(2);
    
    // 创建背包槽位按钮
    createInventorySlots();
    
    scrollArea->setWidget(gridWidget);
    layout->addWidget(scrollArea);
    
    // 操作按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* sortButton = new QPushButton("整理");
    connect(sortButton, &QPushButton::clicked, this, &InventoryWindow::sortInventory);
    buttonLayout->addWidget(sortButton);
    
    QPushButton* clearButton = new QPushButton("清空");
    connect(clearButton, &QPushButton::clicked, this, &InventoryWindow::clearInventory);
    buttonLayout->addWidget(clearButton);
    
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    splitter->addWidget(inventoryWidget);
}

void InventoryWindow::setupDetailArea(QSplitter* splitter)
{
    m_itemDetailPanel = new QGroupBox("物品详情");
    QVBoxLayout* layout = new QVBoxLayout(m_itemDetailPanel);
    
    // 物品图标
    m_itemIconLabel = new QLabel();
    m_itemIconLabel->setFixedSize(64, 64);
    m_itemIconLabel->setAlignment(Qt::AlignCenter);
    m_itemIconLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    layout->addWidget(m_itemIconLabel);
    
    // 物品名称
    m_itemNameLabel = new QLabel("未选择物品");
    m_itemNameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    m_itemNameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_itemNameLabel);
    
    // 物品描述
    m_itemDescriptionEdit = new QTextEdit();
    m_itemDescriptionEdit->setReadOnly(true);
    m_itemDescriptionEdit->setMaximumHeight(150);
    layout->addWidget(m_itemDescriptionEdit);
    
    // 物品属性
    m_itemPropertiesLabel = new QLabel();
    m_itemPropertiesLabel->setWordWrap(true);
    layout->addWidget(m_itemPropertiesLabel);
    
    layout->addStretch();
    
    splitter->addWidget(m_itemDetailPanel);
}

void InventoryWindow::createInventorySlots()
{
    if (!m_inventorySystem) return;
    
    int capacity = m_inventorySystem->getCapacity();
    int columns = 10; // 每行10个槽位
    
    for (int i = 0; i < capacity; ++i) {
        QPushButton* slotButton = new QPushButton();
        slotButton->setFixedSize(50, 50);
        slotButton->setProperty("slotIndex", i);
        
        // 设置样式
        slotButton->setStyleSheet(
            "QPushButton {"
            "    border: 2px solid #999;"
            "    background-color: #f0f0f0;"
            "    border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "    border-color: #666;"
            "    background-color: #e0e0e0;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #d0d0d0;"
            "}"
        );
        
        // 连接点击事件
        connect(slotButton, &QPushButton::clicked, this, &InventoryWindow::onSlotClicked);
        
        int row = i / columns;
        int col = i % columns;
        m_inventoryGrid->addWidget(slotButton, row, col);
        
        m_slotButtons.append(slotButton);
    }
}

void InventoryWindow::setupConnections()
{
    if (!m_inventorySystem) return;
    
    // 连接背包系统信号
    connect(m_inventorySystem, &Game::InventorySystem::itemAdded,
            this, &InventoryWindow::onItemAdded);
    connect(m_inventorySystem, &Game::InventorySystem::itemRemoved,
            this, &InventoryWindow::onItemRemoved);
    connect(m_inventorySystem, &Game::InventorySystem::itemsChanged,
            this, &InventoryWindow::updateInventoryDisplay);
}

void InventoryWindow::updateInventoryDisplay()
{
    if (!m_inventorySystem) return;
    
    // 更新容量显示
    int usedSlots = m_inventorySystem->getUsedSlots();
    int capacity = m_inventorySystem->getCapacity();
    m_capacityLabel->setText(QString("%1/%2").arg(usedSlots).arg(capacity));
    
    // 更新槽位显示
    for (int i = 0; i < m_slotButtons.size(); ++i) {
        updateSlotDisplay(i);
    }
}

void InventoryWindow::updateSlotDisplay(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= m_slotButtons.size() || !m_inventorySystem) {
        return;
    }
    
    QPushButton* button = m_slotButtons[slotIndex];
    QJsonObject slotInfo = m_inventorySystem->getSlotInfo(slotIndex);
    
    if (slotInfo.isEmpty() || slotInfo["itemId"].toInt() == 0) {
        // 空槽位
        button->setText("");
        button->setIcon(QIcon());
        button->setToolTip("");
        button->setStyleSheet(
            "QPushButton {"
            "    border: 2px solid #999;"
            "    background-color: #f0f0f0;"
            "    border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "    border-color: #666;"
            "    background-color: #e0e0e0;"
            "}"
        );
    } else {
        // 有物品的槽位
        int itemId = slotInfo["itemId"].toInt();
        int quantity = slotInfo["quantity"].toInt();
        QString itemName = slotInfo["name"].toString();
        
        button->setText(QString::number(quantity));
        button->setToolTip(QString("%1 x%2").arg(itemName).arg(quantity));
        button->setStyleSheet(
            "QPushButton {"
            "    border: 2px solid #666;"
            "    background-color: #e8f4f8;"
            "    border-radius: 4px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    border-color: #333;"
            "    background-color: #d0e8f0;"
            "}"
        );
        
        // TODO: 设置物品图标
        // button->setIcon(QIcon(iconPath));
    }
}

void InventoryWindow::updateItemDetail(int slotIndex)
{
    if (!m_inventorySystem || slotIndex < 0) {
        // 清空详情显示
        m_itemIconLabel->clear();
        m_itemNameLabel->setText("未选择物品");
        m_itemDescriptionEdit->clear();
        m_itemPropertiesLabel->clear();
        return;
    }
    
    QJsonObject slotInfo = m_inventorySystem->getSlotInfo(slotIndex);
    if (slotInfo.isEmpty() || slotInfo["itemId"].toInt() == 0) {
        // 空槽位
        m_itemIconLabel->clear();
        m_itemNameLabel->setText("空槽位");
        m_itemDescriptionEdit->clear();
        m_itemPropertiesLabel->clear();
        return;
    }
    
    // 显示物品详情
    QString itemName = slotInfo["name"].toString();
    QString description = slotInfo["description"].toString();
    int quantity = slotInfo["quantity"].toInt();
    QString quality = slotInfo["quality"].toString();
    QString type = slotInfo["type"].toString();
    
    m_itemNameLabel->setText(itemName);
    m_itemDescriptionEdit->setText(description);
    
    QString properties = QString(
        "数量: %1\n"
        "品质: %2\n"
        "类型: %3"
    ).arg(quantity).arg(quality).arg(type);
    
    m_itemPropertiesLabel->setText(properties);
    
    // TODO: 设置物品图标
    m_itemIconLabel->setText("📦");
}

void InventoryWindow::onSlotClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    int slotIndex = button->property("slotIndex").toInt();
    
    // 更新选中状态
    if (m_selectedSlot >= 0 && m_selectedSlot < m_slotButtons.size()) {
        // 取消之前的选中状态
        updateSlotDisplay(m_selectedSlot);
    }
    
    m_selectedSlot = slotIndex;
    
    // 设置选中样式
    button->setStyleSheet(button->styleSheet() + "border-color: #ff6600; border-width: 3px;");
    
    // 更新详情显示
    updateItemDetail(slotIndex);
    
    qDebug() << "InventoryWindow: 选中槽位" << slotIndex;
}

void InventoryWindow::onItemAdded(int itemId, int quantity)
{
    qDebug() << "InventoryWindow: 物品添加" << itemId << "x" << quantity;
    updateInventoryDisplay();
}

void InventoryWindow::onItemRemoved(int itemId, int quantity)
{
    qDebug() << "InventoryWindow: 物品移除" << itemId << "x" << quantity;
    updateInventoryDisplay();
}

void InventoryWindow::sortInventory()
{
    if (m_inventorySystem) {
        m_inventorySystem->sortInventory();
        qDebug() << "InventoryWindow: 背包已整理";
    }
}

void InventoryWindow::clearInventory()
{
    if (m_inventorySystem) {
        m_inventorySystem->clearInventory();
        m_selectedSlot = -1;
        updateItemDetail(-1);
        qDebug() << "InventoryWindow: 背包已清空";
    }
}

