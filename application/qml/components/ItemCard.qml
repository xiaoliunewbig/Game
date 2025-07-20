import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: itemCard
    width: 100
    height: 120
    color: "#f39c12"
    radius: 8
    
    Column {
        anchors.centerIn: parent
        spacing: 8
        
        Rectangle {
            width: 48
            height: 48
            color: "#e67e22"
            radius: 4
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Text {
            text: "物品卡片"
            color: "white"
            font.pixelSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}