import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: enemyCard
    width: 120
    height: 80
    color: "#c0392b"
    radius: 8
    
    Text {
        anchors.centerIn: parent
        text: "敌人"
        color: "white"
        font.pixelSize: 14
    }
}