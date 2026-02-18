/*
 * 文件名: InventoryWindow.h
 * 说明: 背包窗口类声明
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */
#pragma once

#include <QWidget>
#include <QJsonObject>
#include <QVector>

class QGridLayout;
class QGroupBox;
class QLabel;
class QTextEdit;
class QPushButton;
class QSplitter;
namespace Game { class InventorySystem; }

class InventoryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWindow(QWidget *parent = nullptr);
    ~InventoryWindow();

private slots:
    void onSlotClicked();
    void onItemAdded(int itemId, int quantity);
    void onItemRemoved(int itemId, int quantity);
    void sortInventory();
    void clearInventory();

private:
    void setupUI();
    void setupInventoryArea(QSplitter* splitter);
    void setupDetailArea(QSplitter* splitter);
    void createInventorySlots();
    void setupConnections();
    void updateInventoryDisplay();
    void updateSlotDisplay(int slotIndex);
    void updateItemDetail(int slotIndex);

    static QString getItemTypeIcon(const QString &type);

    Game::InventorySystem* m_inventorySystem;
    QGridLayout* m_inventoryGrid;
    QGroupBox* m_itemDetailPanel;
    QLabel* m_capacityLabel;
    QLabel* m_itemIconLabel;
    QLabel* m_itemNameLabel;
    QTextEdit* m_itemDescriptionEdit;
    QLabel* m_itemPropertiesLabel;

    QVector<QPushButton*> m_slotButtons;
    int m_selectedSlot;
};
