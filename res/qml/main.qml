import QtQuick 2.2
import QtQuick.Window 2.0
import QtQuick.Controls 1.1
import VisItem 1.0

ApplicationWindow {
    visible: true
    minimumWidth: 800
    minimumHeight: 600
    title: "AmoebotSim"

    VisItem {
        anchors.fill: parent
    }

    Row {
        id: buttonRow

        spacing: 10
        anchors.margins: 10
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        A_Button {
            id: startButton
            text: "start"

            onClicked: {
                if(text == "start") {
                    text = "stop"
                } else {
                    text = "start"
                }
            }
        }

        A_Button {
            id: roundButton
            text: "round"
        }

        A_Button {
            id: backButton
            text: "back"
        }

//        focus: true
//        Keys.onTabPressed: {
//            startButton.visible = !startButton.visible
//            roundButton.visible = !roundButton.visible
//            backButton.visible = !backButton.visible
//        }
    }
}
