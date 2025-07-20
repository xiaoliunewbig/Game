import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: actionButton
    text: "动作"
    
    background: Rectangle {
        color: actionButton.pressed ? "#2980b9" : "#3498db"
        radius: 6
    }
    
    contentItem: Text {
        text: actionButton.text
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}