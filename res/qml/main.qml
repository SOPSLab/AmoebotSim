import QtQuick 2.2
import QtQuick.Window 2.0
import QtQuick.Controls 1.1
import VisItem 1.0

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: 800
    minimumHeight: 600
    title: "AmoebotSim"

    VisItem {
        anchors.fill: parent
    }

    A_ResultTextField {
        id: resultField
        visible: false
        opacity: 0
        anchors.margins: 10
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: window.width - 290

        SequentialAnimation {
            id: resultFieldFade
            running: false

            PauseAnimation {
                duration: 5000
            }

            NumberAnimation {
                target: resultField
                property: "opacity"
                to: 0
                duration: 1000
            }
        }
    }

    Row {
        id: buttonRow
        spacing: 10
        anchors.margins: 10
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        A_TextField {
            id: commandField
            visible: false
            width: window.width - 290

            focus: true
            Keys.onReturnPressed: {
                visible = false

                resultField.text = text
                resultField.opacity = 1
                resultField.visible = true
                resultFieldFade.running = true

                text = ""
                buttonRow.forceActiveFocus()
            }
            Keys.onEscapePressed:{
                visible = false
                text = ""
                buttonRow.forceActiveFocus()
            }
        }

        focus: true
        Keys.onReturnPressed: {
            resultField.visible = false
            resultFieldFade.running = false

            commandField.visible = true
            commandField.forceActiveFocus()
        }

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
    }
}
