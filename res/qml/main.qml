import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import VisItem 1.0

ApplicationWindow {
    id: appWindow
    visible: true
    color: "black"
    minimumWidth: 800
    minimumHeight: 600
    title: "AmoebotSim"

    onWidthChanged: {
        vis.width = width
    }

    onHeightChanged: {
        vis.height = height
    }

    signal start()
    signal stop()
    signal round()
    signal focusOnCenterOfMass()

    signal executeCommand(string cmd)
    signal commandFieldUp()
    signal commandFieldDown()
    signal commandFieldReset()

    function log(msg, isError)
    {
        commandField.visible = false
        buttonRow.forceActiveFocus()

        resultField.text = msg
        resultField.showsError = isError;
        resultField.opacity = 1
        resultField.visible = true
        resultFieldFade.running = true
    }

    function setLabelStart()
    {
        startStopButton.text = "start"
    }

    function setLabelStop()
    {
        startStopButton.text = "stop"
    }

    function setCommand(cmd)
    {
        commandField.text = cmd
    }

    function setNumMovements(num)
    {
        numMovementsText.text = num
    }

    function setRounds(rounds)
    {
        roundsText.text = rounds
    }

    function setResolution(_width, _height){
        if(_width >= appWindow.minimumWidth){
            appWindow.width = _width
        } else {
            appWindow.width = appWindow.minimumWidth
        }

        if(_height >= appWindow.minimumHeight){
            appWindow.height = _height
        } else {
            appWindow.height = appWindow.minimumHeight
        }
    }

    function setFullscreen(){
        showFullScreen()
    }

    function setWindowed(){
        //Check link for values: http://doc.qt.io/qt-5/qwindow.html#Visibility-enum
        if(appWindow.visibility == 5)
            showMaximized()
    }

    VisItem {
        id: vis
    }

    RowLayout{
        id: roundsRow
        anchors.right: vis.right
        anchors.top: vis.top
        anchors.rightMargin: 30
        anchors.topMargin: 10

        Rectangle{
            Layout.preferredWidth: 60
            Text{
                anchors.right: parent.right
                text: "Rounds: "
            }
        }
        Rectangle{
            Layout.preferredWidth: 20
            Text{
                id: roundsText
                anchors.left: parent.left
                text: "0"
            }
        }
    }

    RowLayout{
        id: numMovementsRow
        anchors.top: roundsRow.bottom
        anchors.right: vis.right
        anchors.rightMargin: 30
        anchors.topMargin: 15

        Rectangle{
            Layout.preferredWidth: 60
            Text{
                anchors.right: parent.right
                text: "Movements: "
            }
        }
        Rectangle{
            Layout.preferredWidth: 20
            Text{
                id: numMovementsText
                anchors.left: parent.left
                text: "0"
            }
        }
    }


    Slider {
        id: roundDurationSlider
        objectName: "roundDurationSlider"
        maximumValue: 100.0
        minimumValue: 1.0
        stepSize: 1.0
        updateValueWhileDragging: true
        value: 100.0

        orientation: Qt.Vertical
        implicitHeight: 300
        anchors.bottom: buttonRow.top
        anchors.right: vis.right
        anchors.rightMargin: 25
        anchors.bottomMargin: 10

        signal roundDurationChanged(int value)
        property bool reactOnSet: false;

        onValueChanged: {
            roundDurationChanged(value)
            roundDurationText.text = value + " ms"
        }

        onPressedChanged: {
            reactOnSet = !reactOnSet
        }

        function setRoundDuration(val){
            if(!reactOnSet){
                value = val
            }
        }

        Text{
            id: roundDurationText
            text: ""
            anchors.bottom: roundDurationSlider.top
            anchors.bottomMargin: 10
        }
    }

    Row {
        id: buttonRow
        spacing: 10
        anchors.margins: 10
        anchors.bottom: vis.bottom
        anchors.right: vis.right

        A_TextField {
            id: commandField
            visible: false
            width: startStopButton.visible ? vis.width - 200 : vis.width - 20

            focus: true
            Keys.onPressed: {
                if(event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    if(text != "") {
                        executeCommand(text)
                    }
                    visible = false
                    text = ""
                    buttonRow.forceActiveFocus()
                    event.accepted = true
                } else if(event.key === Qt.Key_Escape) {
                    visible = false
                    text = ""
                    buttonRow.forceActiveFocus()
                    commandFieldReset()
                    event.accepted = true
                } else if(event.key === Qt.Key_Up) {
                    commandFieldUp()
                    event.accepted = true
                } else if(event.key === Qt.Key_Down) {
                    commandFieldDown()
                    event.accepted = true
                }
            }
        }

        A_ResultTextField {
            id: resultField
            visible: false
            opacity: 0
            width: startStopButton.visible ? vis.width - 200 : vis.width - 20

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

        focus: true
        property int ui_state: 0

        Keys.onPressed: {
            if(event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                resultField.visible = false
                resultFieldFade.running = false
                commandField.visible = true
                commandField.forceActiveFocus()
                event.accepted = true
            } else if(event.key === Qt.Key_B && (event.modifiers & Qt.ControlModifier)) {
                ui_state++
                if (ui_state >= 3)
                    ui_state = 0

                if (ui_state == 0){
                    startStopButton.visible = true
                    roundButton.visible = true
                    roundDurationSlider.visible = true
                    roundsRow.visible = true
                    numMovementsRow.visible = true
                }else if (ui_state == 1){
                    roundDurationSlider.visible = false
                    roundsRow.visible = false
                    numMovementsRow.visible = false
                }else if (ui_state == 2){
                    startStopButton.visible = false
                    roundButton.visible = false
                }
                event.accepted = true
            } else if(event.key === Qt.Key_E && (event.modifiers & Qt.ControlModifier)) {
                if(startStopButton.text === "start") {
                    start()
                } else {
                    stop()
                }
                event.accepted = true
            } else if(event.key === Qt.Key_D && (event.modifiers & Qt.ControlModifier)) {
                round()
                event.accepted = true
            } else if(event.key === Qt.Key_F && (event.modifiers & Qt.ControlModifier)) {
                vis.focusOnCenterOfMass()
                event.accepted = true
            }
        }

        A_Button {
            id: startStopButton
            text: "start"

            onClicked: {
                if(text == "start") {
                    start()
                } else {
                    stop()
                }
            }
        }

        A_Button {
            id: roundButton
            text: "round"
            onClicked: {
                round()
            }
        }

//        A_Button {
//            id: backButton
//            text: "back"
//            onClicked: {
//                back()
//            }
//        }
    }
}
