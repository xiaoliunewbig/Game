/*
 * 文件名: InventorySystem.cpp
 * 说明: 背包系统实现文件
 * 作者: 彭承康
 * 创建时间: 2025-07-24
 */

#include "game/InventorySystem.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <algorithm>

namespace Game {

InventorySystem::InventorySystem(QObject* parent)
    : QObject(parent)
    , m_capacity(50)  // 默认50个槽位
{
    // 初始化背包槽位
    m_slots.resize(m_capacity);
    
    // 设置数据文件路径
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_inventoryFilePath = dataDir + "/inventory.json";
    m_itemDataFilePath = dataDir + "/items.json";
    
    // 初始化物品数据库
    initializeItemDatabase();
    
    // 加载背包数据
    loadInventory();
    
    qDebug() << "InventorySystem: 背包系统初始化完成，容量:" << m_capacity;
}

InventorySystem::~InventorySystem()
{
    // 保存背包数据
    saveInventory();
    qDebug() << "InventorySystem: 背包系统销毁";
}

bool InventorySystem::addItem(int itemId, int quantity)
{
    if (itemId <= 0 || quantity <= 0) {
        qWarning() << "InventorySystem: 无效的物品ID或数量:" << itemId << quantity;
        return false;
    }
    
    ItemData itemData = getItemData(itemId);
    if (itemData.id == 0) {
        qWarning() << "InventorySystem: 未找到物品数据:" << itemId;
        return false;
    }
    
    int remainingQuantity = quantity;
    
    // 首先尝试添加到现有堆叠
    if (itemData.maxStack > 1) {
        QList<int> existingSlots = findItemSlots(itemId);
        for (int slotIndex : existingSlots) {
            InventorySlot& slot = m_slots[slotIndex];
            int canAdd = itemData.maxStack - slot.quantity;
            if (canAdd > 0) {
                int addAmount = qMin(canAdd, remainingQuantity);
                slot.quantity += addAmount;
                remainingQuantity -= addAmount;
                
                if (remainingQuantity == 0) {
                    break;
                }
            }
        }
    }
    
    // 如果还有剩余，添加到新槽位
    while (remainingQuantity > 0) {
        int emptySlot = findEmptySlot();
        if (emptySlot == -1) {
            qWarning() << "InventorySystem: 背包已满，无法添加物品:" << itemId;
            emit inventoryFull();
            return false;
        }
        
        int addAmount = qMin(remainingQuantity, itemData.maxStack);
        m_slots[emptySlot] = InventorySlot(itemId, addAmount);
        remainingQuantity -= addAmount;
    }
    
    updateInventoryState();
    emit itemAdded(itemId, quantity);
    
    qDebug() << "InventorySystem: 添加物品成功:" << itemData.name << "x" << quantity;
    return true;
}

bool InventorySystem::removeItem(int itemId, int quantity)
{
    if (itemId <= 0 || quantity <= 0) {
        return false;
    }
    
    if (!hasItem(itemId, quantity)) {
        qWarning() << "InventorySystem: 物品数量不足:" << itemId << quantity;
        return false;
    }
    
    int remainingQuantity = quantity;
    QList<int> itemSlots = findItemSlots(itemId);
    
    // 从后往前移除，避免索引问题
    for (int i = itemSlots.size() - 1; i >= 0 && remainingQuantity > 0; --i) {
        int slotIndex = itemSlots[i];
        InventorySlot& slot = m_slots[slotIndex];
        
        int removeAmount = qMin(slot.quantity, remainingQuantity);
        slot.quantity -= removeAmount;
        remainingQuantity -= removeAmount;
        
        // 如果槽位空了，清空
        if (slot.quantity == 0) {
            slot.itemId = 0;
        }
    }
    
    updateInventoryState();
    emit itemRemoved(itemId, quantity);
    
    qDebug() << "InventorySystem: 移除物品成功:" << itemId << "x" << quantity;
    return true;
}

bool InventorySystem::useItem(int slotIndex, int quantity)
{
    if (!isValidSlot(slotIndex) || quantity <= 0) {
        return false;
    }
    
    InventorySlot& slot = m_slots[slotIndex];
    if (slot.isEmpty() || slot.quantity < quantity) {
        return false;
    }
    
    ItemData itemData = getItemData(slot.itemId);
    if (itemData.type != ItemType::Consumable) {
        qWarning() << "InventorySystem: 物品不可使用:" << itemData.name;
        return false;
    }
    
    // 执行物品使用效果
    // TODO: 实现具体的物品效果
    
    // 减少物品数量
    slot.quantity -= quantity;
    if (slot.quantity == 0) {
        slot.itemId = 0;
    }
    
    updateInventoryState();
    emit itemUsed(slot.itemId, quantity);
    
    qDebug() << "InventorySystem: 使用物品:" << itemData.name << "x" << quantity;
    return true;
}

bool InventorySystem::moveItem(int fromSlot, int toSlot)
{
    if (!isValidSlot(fromSlot) || !isValidSlot(toSlot) || fromSlot == toSlot) {
        return false;
    }
    
    InventorySlot& from = m_slots[fromSlot];
    InventorySlot& to = m_slots[toSlot];
    
    if (from.isEmpty()) {
        return false;
    }
    
    // 如果目标槽位为空，直接移动
    if (to.isEmpty()) {
        to = from;
        from = InventorySlot();
        updateInventoryState();
        return true;
    }
    
    // 如果是相同物品，尝试合并
    if (from.itemId == to.itemId) {
        ItemData itemData = getItemData(from.itemId);
        int canMerge = itemData.maxStack - to.quantity;
        if (canMerge > 0) {
            int mergeAmount = qMin(canMerge, from.quantity);
            to.quantity += mergeAmount;
            from.quantity -= mergeAmount;
            
            if (from.quantity == 0) {
                from = InventorySlot();
            }
            
            updateInventoryState();
            return true;
        }
    }
    
    // 否则交换位置
    InventorySlot temp = from;
    from = to;
    to = temp;
    
    updateInventoryState();
    return true;
}

int InventorySystem::splitItem(int slotIndex, int quantity)
{
    if (!isValidSlot(slotIndex) || quantity <= 0) {
        return -1;
    }
    
    InventorySlot& slot = m_slots[slotIndex];
    if (slot.isEmpty() || slot.quantity <= quantity) {
        return -1;
    }
    
    int emptySlot = findEmptySlot();
    if (emptySlot == -1) {
        return -1;
    }
    
    // 分割物品
    slot.quantity -= quantity;
    m_slots[emptySlot] = InventorySlot(slot.itemId, quantity);
    
    updateInventoryState();
    return emptySlot;
}

int InventorySystem::getUsedSlots() const
{
    int count = 0;
    for (const InventorySlot& slot : m_slots) {
        if (!slot.isEmpty()) {
            count++;
        }
    }
    return count;
}

int InventorySystem::getItemCount(int itemId) const
{
    int totalCount = 0;
    for (const InventorySlot& slot : m_slots) {
        if (slot.itemId == itemId) {
            totalCount += slot.quantity;
        }
    }
    return totalCount;
}

bool InventorySystem::hasItem(int itemId, int quantity) const
{
    return getItemCount(itemId) >= quantity;
}

QJsonObject InventorySystem::getSlotInfo(int slotIndex) const
{
    QJsonObject slotInfo;
    
    if (!isValidSlot(slotIndex)) {
        return slotInfo;
    }
    
    const InventorySlot& slot = m_slots[slotIndex];
    slotInfo["slotIndex"] = slotIndex;
    slotInfo["isEmpty"] = slot.isEmpty();
    
    if (!slot.isEmpty()) {
        slotInfo["itemId"] = slot.itemId;
        slotInfo["quantity"] = slot.quantity;
        
        ItemData itemData = getItemData(slot.itemId);
        slotInfo["itemName"] = itemData.name;
        slotInfo["itemType"] = static_cast<int>(itemData.type);
        slotInfo["itemQuality"] = static_cast<int>(itemData.quality);
        slotInfo["iconPath"] = itemData.iconPath;
        slotInfo["maxStack"] = itemData.maxStack;
    }
    
    return slotInfo;
}

QJsonObject InventorySystem::getItemInfo(int itemId) const
{
    QJsonObject itemInfo;
    ItemData itemData = getItemData(itemId);
    
    if (itemData.id == 0) {
        return itemInfo;
    }
    
    itemInfo["id"] = itemData.id;
    itemInfo["name"] = itemData.name;
    itemInfo["description"] = itemData.description;
    itemInfo["type"] = static_cast<int>(itemData.type);
    itemInfo["quality"] = static_cast<int>(itemData.quality);
    itemInfo["maxStack"] = itemData.maxStack;
    itemInfo["iconPath"] = itemData.iconPath;
    itemInfo["properties"] = itemData.properties;
    itemInfo["ownedCount"] = getItemCount(itemId);
    
    return itemInfo;
}

QJsonArray InventorySystem::getItemsByType(const QString& type) const
{
    QJsonArray result;
    ItemType itemType = static_cast<ItemType>(type.toInt());
    
    for (const InventorySlot& slot : m_slots) {
        if (!slot.isEmpty()) {
            ItemData itemData = getItemData(slot.itemId);
            if (itemData.type == itemType) {
                result.append(getSlotInfo(&slot - &m_slots[0]));
            }
        }
    }
    
    return result;
}

QJsonArray InventorySystem::getItemsByQuality(const QString& quality) const
{
    QJsonArray result;
    ItemQuality itemQuality = static_cast<ItemQuality>(quality.toInt());
    
    for (const InventorySlot& slot : m_slots) {
        if (!slot.isEmpty()) {
            ItemData itemData = getItemData(slot.itemId);
            if (itemData.quality == itemQuality) {
                result.append(getSlotInfo(&slot - &m_slots[0]));
            }
        }
    }
    
    return result;
}

QJsonArray InventorySystem::searchItems(const QString& keyword) const
{
    QJsonArray result;
    QString lowerKeyword = keyword.toLower();
    
    for (int i = 0; i < m_slots.size(); ++i) {
        const InventorySlot& slot = m_slots[i];
        if (!slot.isEmpty()) {
            ItemData itemData = getItemData(slot.itemId);
            if (itemData.name.toLower().contains(lowerKeyword) ||
                itemData.description.toLower().contains(lowerKeyword)) {
                result.append(getSlotInfo(i));
            }
        }
    }
    
    return result;
}

QStringList InventorySystem::getCategories() const
{
    return QStringList() << "武器" << "护甲" << "消耗品" << "材料" << "任务物品" << "杂项";
}

void InventorySystem::setCapacity(int capacity)
{
    if (capacity <= 0 || capacity == m_capacity) {
        return;
    }
    
    if (capacity < m_capacity) {
        // 缩小容量时检查是否有物品会丢失
        for (int i = capacity; i < m_capacity; ++i) {
            if (!m_slots[i].isEmpty()) {
                qWarning() << "InventorySystem: 无法缩小容量，槽位" << i << "不为空";
                return;
            }
        }
    }
    
    m_capacity = capacity;
    m_slots.resize(capacity);
    
    emit capacityChanged(capacity);
    updateInventoryState();
    
    qDebug() << "InventorySystem: 背包容量设置为:" << capacity;
}

bool InventorySystem::saveInventory(const QString& filePath) const
{
    QString path = filePath.isEmpty() ? m_inventoryFilePath : filePath;
    
    QJsonObject inventoryData;
    inventoryData["capacity"] = m_capacity;
    inventoryData["version"] = "1.0";
    
    QJsonArray slotsArray;
    for (const InventorySlot& slot : m_slots) {
        QJsonObject slotObj;
        slotObj["itemId"] = slot.itemId;
        slotObj["quantity"] = slot.quantity;
        slotsArray.append(slotObj);
    }
    inventoryData["slots"] = slotsArray;
    
    QJsonDocument doc(inventoryData);
    QFile file(path);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "InventorySystem: 无法保存背包数据到:" << path;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "InventorySystem: 背包数据保存成功:" << path;
    return true;
}

bool InventorySystem::loadInventory(const QString& filePath)
{
    QString path = filePath.isEmpty() ? m_inventoryFilePath : filePath;
    
    QFile file(path);
    if (!file.exists()) {
        qDebug() << "InventorySystem: 背包数据文件不存在，使用默认设置";
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "InventorySystem: 无法读取背包数据:" << path;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "InventorySystem: 背包数据解析失败:" << error.errorString();
        return false;
    }
    
    QJsonObject inventoryData = doc.object();
    
    // 加载容量
    int capacity = inventoryData["capacity"].toInt(50);
    if (capacity != m_capacity) {
        setCapacity(capacity);
    }
    
    // 加载槽位数据
    QJsonArray slotsArray = inventoryData["slots"].toArray();
    for (int i = 0; i < slotsArray.size() && i < m_capacity; ++i) {
        QJsonObject slotObj = slotsArray[i].toObject();
        m_slots[i].itemId = slotObj["itemId"].toInt();
        m_slots[i].quantity = slotObj["quantity"].toInt();
    }
    
    updateInventoryState();
    qDebug() << "InventorySystem: 背包数据加载成功";
    return true;
}

void InventorySystem::clearInventory()
{
    for (InventorySlot& slot : m_slots) {
        slot = InventorySlot();
    }
    
    updateInventoryState();
    qDebug() << "InventorySystem: 背包已清空";
}

void InventorySystem::sortInventory()
{
    // 收集所有非空物品
    QList<InventorySlot> items;
    for (const InventorySlot& slot : m_slots) {
        if (!slot.isEmpty()) {
            items.append(slot);
        }
    }
    
    // 按物品ID排序
    std::sort(items.begin(), items.end(), [](const InventorySlot& a, const InventorySlot& b) {
        return a.itemId < b.itemId;
    });
    
    // 清空背包
    for (InventorySlot& slot : m_slots) {
        slot = InventorySlot();
    }
    
    // 重新放置物品
    int slotIndex = 0;
    for (const InventorySlot& item : items) {
        if (slotIndex >= m_capacity) break;
        m_slots[slotIndex++] = item;
    }
    
    updateInventoryState();
    qDebug() << "InventorySystem: 背包整理完成";
}

QJsonArray InventorySystem::getItemsJson() const
{
    QJsonArray itemsArray;
    
    for (int i = 0; i < m_slots.size(); ++i) {
        itemsArray.append(getSlotInfo(i));
    }
    
    return itemsArray;
}

void InventorySystem::initializeItemDatabase()
{
    // 加载物品数据
    if (!loadDefaultItemData()) {
        // 如果加载失败，创建一些默认物品
        loadDefaultItemData();
    }
    
    qDebug() << "InventorySystem: 物品数据库初始化完成，物品数量:" << m_itemDatabase.size();
}

void InventorySystem::reloadItemData()
{
    m_itemDatabase.clear();
    initializeItemDatabase();
    updateInventoryState();
}

int InventorySystem::findEmptySlot() const
{
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

QList<int> InventorySystem::findItemSlots(int itemId) const
{
    QList<int> slots;
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].itemId == itemId) {
            slots.append(i);
        }
    }
    return slots;
}

ItemData InventorySystem::getItemData(int itemId) const
{
    return m_itemDatabase.value(itemId, ItemData());
}

bool InventorySystem::isValidSlot(int slotIndex) const
{
    return slotIndex >= 0 && slotIndex < m_capacity;
}

void InventorySystem::updateInventoryState()
{
    emit usedSlotsChanged(getUsedSlots());
    emit itemsChanged();
}

void InventorySystem::loadDefaultItemData()
{
    // 创建一些默认物品数据
    ItemData healthPotion;
    healthPotion.id = 1001;
    healthPotion.name = "生命药水";
    healthPotion.description = "恢复50点生命值";
    healthPotion.type = ItemType::Consumable;
    healthPotion.quality = ItemQuality::Common;
    healthPotion.maxStack = 10;
    healthPotion.iconPath = ":/resources/icons/health_potion.png";
    m_itemDatabase[1001] = healthPotion;
    
    ItemData manaPotion;
    manaPotion.id = 1002;
    manaPotion.name = "魔法药水";
    manaPotion.description = "恢复30点魔法值";
    manaPotion.type = ItemType::Consumable;
    manaPotion.quality = ItemQuality::Common;
    manaPotion.maxStack = 10;
    manaPotion.iconPath = ":/resources/icons/mana_potion.png";
    m_itemDatabase[1002] = manaPotion;
    
    ItemData ironSword;
    ironSword.id = 2001;
    ironSword.name = "铁剑";
    ironSword.description = "一把普通的铁制长剑";
    ironSword.type = ItemType::Weapon;
    ironSword.quality = ItemQuality::Common;
    ironSword.maxStack = 1;
    ironSword.iconPath = ":/resources/icons/iron_sword.png";
    m_itemDatabase[2001] = ironSword;
    
    qDebug() << "InventorySystem: 默认物品数据加载完成";
}

} // namespace Game

