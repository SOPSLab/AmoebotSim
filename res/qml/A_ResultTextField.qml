import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

TextField {
    enabled: false
    textColor: "black"
    style: TextFieldStyle {
        background: Rectangle {
            implicitWidth: 260
            implicitHeight: 25
            border.width: 1
            border.color: "#888"
            radius: 4
            color: "#4d4"
            opacity: 0.75
        }
    }
}
