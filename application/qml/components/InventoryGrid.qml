import QtQuick 2.15
import QtQuick.Controls 2.15

GridView {
    id: inventoryGrid
    cellWidth: 70
    cellHeight: 70
    
    model: 20
    
    delegate: Rectangle {
        width: 64
        height: 64
        color: "#bdc3c7"
        border.color: "#95a5a6"
        border.width: 1
        radius: 4
        
        Text {
            anchors.centerIn: parent
            text: index + 1
            color: "#2c3e50"
        }
    }
}