/*
 * 文件名: InventorySystem.h
 * 说明: 背包系统头文件 - 管理玩家物品和背包功能
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 * 版本: v1.0.0
 * 
 * 功能描述:
 * - 物品的添加、移除和使用
 * - 背包容量管理
 * - 物品分类和筛选
 * - 物品属性查询
 * - 背包状态持久化
 * 
 * 设计特点:
 * - 事件驱动：通过Qt信号槽通知背包变化
 * - 类型安全：使用枚举定义物品类型
 * - 高效查询：使用哈希表存储物品数据
 * - 扩展性：支持自定义物品属性
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

namespace Game {

/**
 * @brief 物品类型枚举
 */
enum class ItemType {
    Weapon,         ///< 武器
    Armor,          ///< 护甲
    Consumable,     ///< 消耗品
    Material,       ///< 材料
    Quest,          ///< 任务物品
    Misc            ///< 杂项
};

/**
 * @brief 物品品质枚举
 */
enum class ItemQuality {
    Common,         ///< 普通（白色）
    Uncommon,       ///< 不常见（绿色）
    Rare,           ///< 稀有（蓝色）
    Epic,           ///< 史诗（紫色）
    Legendary       ///< 传说（橙色）
};

/**
 * @brief 物品数据结构
 */
struct ItemData {
    int id;                     ///< 物品ID
    QString name;               ///< 物品名称
    QString description;        ///< 物品描述
    ItemType type;              ///< 物品类型
    ItemQuality quality;        ///< 物品品质
    int maxStack;               ///< 最大堆叠数量
    QString iconPath;           ///< 图标路径
    QJsonObject properties;     ///< 自定义属性
    
    ItemData() : id(0), type(ItemType::Misc), quality(ItemQuality::Common), maxStack(1) {}
};

/**
 * @brief 背包槽位结构
 */
struct InventorySlot {
    int itemId;                 ///< 物品ID（0表示空槽位）
    int quantity;               ///< 数量
    
    InventorySlot() : itemId(0), quantity(0) {}
    InventorySlot(int id, int qty) : itemId(id), quantity(qty) {}
    
    bool isEmpty() const { return itemId == 0 || quantity == 0; }
};

/**
 * @brief 背包系统类
 * 
 * 管理玩家的背包和物品系统，提供完整的物品管理功能。
 * 支持物品的增删改查、分类筛选、使用消耗等操作。
 */
class InventorySystem : public QObject
{
    Q_OBJECT
    
    // QML可访问的属性
    Q_PROPERTY(int capacity READ getCapacity WRITE setCapacity NOTIFY capacityChanged)
    Q_PROPERTY(int usedSlots READ getUsedSlots NOTIFY usedSlotsChanged)
    Q_PROPERTY(QJsonArray items READ getItemsJson NOTIFY itemsChanged)
    Q_PROPERTY(QStringList categories READ getCategories NOTIFY categoriesChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit InventorySystem(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~InventorySystem();

    // ==================== 基础背包操作 ====================
    
    /**
     * @brief 添加物品到背包
     * @param itemId 物品ID
     * @param quantity 数量
     * @return bool 是否添加成功
     */
    Q_INVOKABLE bool addItem(int itemId, int quantity = 1);
    
    /**
     * @brief 从背包移除物品
     * @param itemId 物品ID
     * @param quantity 数量
     * @return bool 是否移除成功
     */
    Q_INVOKABLE bool removeItem(int itemId, int quantity = 1);
    
    /**
     * @brief 使用物品
     * @param slotIndex 槽位索引
     * @param quantity 使用数量
     * @return bool 是否使用成功
     */
    Q_INVOKABLE bool useItem(int slotIndex, int quantity = 1);
    
    /**
     * @brief 移动物品到指定槽位
     * @param fromSlot 源槽位
     * @param toSlot 目标槽位
     * @return bool 是否移动成功
     */
    Q_INVOKABLE bool moveItem(int fromSlot, int toSlot);
    
    /**
     * @brief 分割物品堆叠
     * @param slotIndex 槽位索引
     * @param quantity 分割数量
     * @return int 新槽位索引，-1表示失败
     */
    Q_INVOKABLE int splitItem(int slotIndex, int quantity);

    // ==================== 查询接口 ====================
    
    /**
     * @brief 获取背包容量
     * @return int 背包容量
     */
    int getCapacity() const { return m_capacity; }
    
    /**
     * @brief 获取已使用槽位数
     * @return int 已使用槽位数
     */
    int getUsedSlots() const;
    
    /**
     * @brief 获取物品数量
     * @param itemId 物品ID
     * @return int 物品总数量
     */
    Q_INVOKABLE int getItemCount(int itemId) const;
    
    /**
     * @brief 检查是否有足够的物品
     * @param itemId 物品ID
     * @param quantity 需要数量
     * @return bool 是否有足够数量
     */
    Q_INVOKABLE bool hasItem(int itemId, int quantity = 1) const;
    
    /**
     * @brief 获取槽位信息
     * @param slotIndex 槽位索引
     * @return QJsonObject 槽位信息
     */
    Q_INVOKABLE QJsonObject getSlotInfo(int slotIndex) const;
    
    /**
     * @brief 获取物品信息
     * @param itemId 物品ID
     * @return QJsonObject 物品信息
     */
    Q_INVOKABLE QJsonObject getItemInfo(int itemId) const;

    // ==================== 筛选和分类 ====================
    
    /**
     * @brief 按类型筛选物品
     * @param type 物品类型
     * @return QJsonArray 筛选结果
     */
    Q_INVOKABLE QJsonArray getItemsByType(const QString& type) const;
    
    /**
     * @brief 按品质筛选物品
     * @param quality 物品品质
     * @return QJsonArray 筛选结果
     */
    Q_INVOKABLE QJsonArray getItemsByQuality(const QString& quality) const;
    
    /**
     * @brief 搜索物品
     * @param keyword 关键词
     * @return QJsonArray 搜索结果
     */
    Q_INVOKABLE QJsonArray searchItems(const QString& keyword) const;
    
    /**
     * @brief 获取所有物品类别
     * @return QStringList 类别列表
     */
    QStringList getCategories() const;

    // ==================== 配置和持久化 ====================
    
    /**
     * @brief 设置背包容量
     * @param capacity 新容量
     */
    void setCapacity(int capacity);
    
    /**
     * @brief 保存背包数据
     * @param filePath 保存路径
     * @return bool 是否保存成功
     */
    bool saveInventory(const QString& filePath = QString()) const;
    
    /**
     * @brief 加载背包数据
     * @param filePath 加载路径
     * @return bool 是否加载成功
     */
    bool loadInventory(const QString& filePath = QString());
    
    /**
     * @brief 清空背包
     */
    Q_INVOKABLE void clearInventory();
    
    /**
     * @brief 整理背包
     * 
     * 自动合并相同物品，移除空槽位
     */
    Q_INVOKABLE void sortInventory();

    // ==================== QML接口 ====================
    
    /**
     * @brief 获取物品JSON数组（供QML使用）
     * @return QJsonArray 物品数据
     */
    QJsonArray getItemsJson() const;

public slots:
    /**
     * @brief 初始化物品数据库
     */
    void initializeItemDatabase();
    
    /**
     * @brief 重新加载物品数据
     */
    void reloadItemData();

signals:
    /**
     * @brief 背包容量变化信号
     * @param newCapacity 新容量
     */
    void capacityChanged(int newCapacity);
    
    /**
     * @brief 已使用槽位数变化信号
     * @param usedSlots 已使用槽位数
     */
    void usedSlotsChanged(int usedSlots);
    
    /**
     * @brief 物品变化信号
     */
    void itemsChanged();
    
    /**
     * @brief 类别变化信号
     */
    void categoriesChanged();
    
    /**
     * @brief 物品添加信号
     * @param itemId 物品ID
     * @param quantity 数量
     */
    void itemAdded(int itemId, int quantity);
    
    /**
     * @brief 物品移除信号
     * @param itemId 物品ID
     * @param quantity 数量
     */
    void itemRemoved(int itemId, int quantity);
    
    /**
     * @brief 物品使用信号
     * @param itemId 物品ID
     * @param quantity 数量
     */
    void itemUsed(int itemId, int quantity);
    
    /**
     * @brief 背包已满信号
     */
    void inventoryFull();

    /**
     * @brief 物品使用结果信号
     * @param itemId 物品ID
     * @param quantity 使用数量
     * @param success 是否使用成功
     */
    void itemUseResult(int itemId, int quantity, bool success);  

private:
    /**
     * @brief 查找空槽位
     * @return int 空槽位索引，-1表示没有空槽位
     */
    int findEmptySlot() const;
    
    /**
     * @brief 查找物品槽位
     * @param itemId 物品ID
     * @return QList<int> 包含该物品的槽位列表
     */
    QList<int> findItemSlots(int itemId) const;
    
    /**
     * @brief 获取物品数据
     * @param itemId 物品ID
     * @return ItemData 物品数据
     */
    ItemData getItemData(int itemId) const;
    
    /**
     * @brief 验证槽位索引
     * @param slotIndex 槽位索引
     * @return bool 是否有效
     */
    bool isValidSlot(int slotIndex) const;
    
    /**
     * @brief 更新背包状态
     */
    void updateInventoryState();
    
    /**
     * @brief 加载默认物品数据
     */
    void loadDefaultItemData();

    /**
     * @brief 应用物品使用效果
     * @param itemData 物品数据
     * @return bool 效果是否成功应用
     */
    bool applyItemEffect(const ItemData &itemData);

private:
    /**
     * @brief 背包容量
     */
    int m_capacity;
    
    /**
     * @brief 背包槽位数组
     */
    QVector<InventorySlot> m_slots;
    
    /**
     * @brief 物品数据库
     */
    QMap<int, ItemData> m_itemDatabase;
    
    /**
     * @brief 背包数据文件路径
     */
    QString m_inventoryFilePath;
    
    /**
     * @brief 物品数据文件路径
     */
    QString m_itemDataFilePath;
};

} // namespace Game
