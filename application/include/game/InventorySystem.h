#pragma once
#include <QObject>
namespace Game {
class InventorySystem : public QObject {
    Q_OBJECT
public:
    explicit InventorySystem(QObject* parent = nullptr);
    ~InventorySystem();
};
}