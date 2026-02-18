/*
 * 文件名: GameplayWindow.h
 * 说明: 游戏玩法窗口类声明
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */
#pragma once

#include <QWidget>
#include <QJsonObject>

class QHBoxLayout;
class QVBoxLayout;
class QGroupBox;
class QProgressBar;
class QLabel;
class QTextEdit;
class QLineEdit;
class QTimer;
class ConfigManager;
class PerformanceMonitor;
namespace Game { class InventorySystem; }

enum class ChatMessageType {
    System = 0,
    Player = 1,
    NPC = 2,
    Combat = 3
};

class GameplayWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameplayWindow(QWidget *parent = nullptr);
    ~GameplayWindow();

    void updatePlayerInfo(const QString &name, int level,
                          int health, int maxHealth,
                          int mana, int maxMana,
                          int experience, int maxExperience);

    void addChatMessage(const QString &sender, const QString &message,
                        ChatMessageType type = ChatMessageType::System);

signals:
    void inventoryToggled(bool visible);
    void chatToggled(bool visible);
    void skillPanelToggled(bool visible);
    void miniMapToggled(bool visible);
    void gameMenuRequested();
    void skillUsed(int skillIndex);
    void inventorySlotClicked(int slotIndex, const QJsonObject &slotInfo);
    void chatMessageSent(const QString &message);
    void leftClicked(const QPoint &position);
    void rightClicked(const QPoint &position);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void updateUI();
    void onSkillButtonClicked(int skillIndex);
    void onInventorySlotClicked(int slotIndex);
    void onSortInventoryClicked();
    void onSendChatMessage();
    void onItemAdded(int itemId, int quantity);
    void onItemRemoved(int itemId, int quantity);
    void onInventoryFull();

private:
    void initializeComponents();
    void setupUI();
    void setupLeftPanel(QHBoxLayout* mainLayout);
    void setupCentralArea(QHBoxLayout* mainLayout);
    void setupRightPanel(QHBoxLayout* mainLayout);
    void setupPlayerInfoPanel(QVBoxLayout* layout);
    void setupSkillPanel(QVBoxLayout* layout);
    void setupMiniMapPanel(QVBoxLayout* layout);
    void setupInventoryPanel(QVBoxLayout* layout);
    void setupChatPanel(QVBoxLayout* layout);
    void setupConnections();
    void applyStyles();
    void startUpdateTimer();

    void toggleInventory();
    void toggleChat();
    void toggleSkillPanel();
    void toggleMiniMap();
    void showGameMenu();
    void focusChat();
    void handleLeftClick(const QPoint &position);
    void handleRightClick(const QPoint &position);

    ConfigManager* m_configManager;
    Game::InventorySystem* m_inventorySystem;
    PerformanceMonitor* m_performanceMonitor;

    QWidget* m_gameView;
    QGroupBox* m_playerInfoPanel;
    QGroupBox* m_inventoryPanel;
    QGroupBox* m_chatPanel;
    QGroupBox* m_miniMapPanel;
    QGroupBox* m_skillPanel;

    QProgressBar* m_healthBar;
    QProgressBar* m_manaBar;
    QProgressBar* m_experienceBar;

    QLabel* m_playerNameLabel;
    QLabel* m_playerLevelLabel;

    QLineEdit* m_chatTextEdit;
    QTextEdit* m_logTextEdit;

    QTimer* m_updateTimer;

    bool m_isInventoryVisible;
    bool m_isChatVisible;
    bool m_isSkillPanelVisible;
};
