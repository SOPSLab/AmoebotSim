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

  function log(msg, isError) {
    commandField.visible = false
    buttonRow.forceActiveFocus()

    resultField.text = msg
    resultField.showsError = isError;
    resultField.opacity = 1
    resultField.visible = true
    resultFieldFade.running = true
  }

  function setLabelStart() {
    startStopButton.text = "start"
  }

  function setLabelStop() {
    startStopButton.text = "stop"
  }

  function setCommand(cmd) {
    commandField.text = cmd
  }

  function setNumMovements(num) {
    numMovementsText.text = num
  }

  function setNumRounds(num) {
    numRoundsText.text = num
  }

  function setResolution(_width, _height) {
    if (_width >= appWindow.minimumWidth) {
      appWindow.width = _width
    } else {
      appWindow.width = appWindow.minimumWidth
    }

    if (_height >= appWindow.minimumHeight) {
      appWindow.height = _height
    } else {
      appWindow.height = appWindow.minimumHeight
    }
  }

  function inspectParticle(text) {
    inspectorText.text = text
    if (text !== "") {
      inspector.visible = true
    } else {
      inspector.visible = false
    }
  }

  VisItem {
    id: vis
  }

  A_Inspector {
    id: inspector
    visible: false
    anchors.top: vis.top
    anchors.left: vis.left
    anchors.margins: 10
    height: inspectorText.height + 20

    Text {
      id: inspectorText
      anchors.margins: 10
      anchors.top: parent.top
      anchors.left: parent.left
      color: "#000"
      text: ""
    }
  }

  RowLayout {
    id: roundsRow
    anchors.top: vis.top
    anchors.topMargin: 10
    anchors.right: vis.right
    anchors.rightMargin: 30

    Rectangle {
      Layout.preferredWidth: 30

      Text {
        anchors.right: parent.right
        text: "Rounds:"
      }
    }

    Rectangle {
      Layout.preferredWidth: 25

      Text {
        id: numRoundsText
        anchors.left: parent.left
        text: "0"
      }
    }
  }

  RowLayout {
    id: numMovementsRow
    anchors.top: roundsRow.bottom
    anchors.topMargin: 25
    anchors.right: vis.right
    anchors.rightMargin: 30

    Rectangle {
      Layout.preferredWidth: 30

      Text {
        anchors.right: parent.right
        text: "Moves:"
      }
    }

    Rectangle {
      Layout.preferredWidth: 25

      Text {
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
    stepSize: 0.0
    updateValueWhileDragging: true
    value: 100.0

    orientation: Qt.Vertical
    implicitHeight: 300
    anchors.bottom: buttonRow.top
    anchors.right: vis.right
    anchors.rightMargin: 25
    anchors.bottomMargin: 10

    signal roundDurationChanged(int value)
    property bool setterDisabled: false;
    property bool callbackDisabled: false;

    onValueChanged: {
      if (!callbackDisabled){
        roundDurationChanged(transferFunc(value))
        roundDurationText.text = transferFunc(value) + " ms"
      }
    }

    onPressedChanged: {
      // When changing the value via slider disable the "setRoundDuration"
      // function because it is always called when "value" changes. This is
      // because "onValueChanged" calls "roundDurationChanged" which results in
      // a "setRoundDuration" call. Therefore we break the cycle by disabling
      // the latter.
      setterDisabled = !setterDisabled
    }

    function setRoundDuration(ms){
      if (!setterDisabled){
        // When setting the ms value via console this setter is called. This
        // setter changes the value of the slider which results in a call of
        // "onValueChanged". As explained above, that function results in a call
        // of this setter again. Therefore we break the cycle by disabling the
        // callback "onValueChanged" for this value change.
        callbackDisabled = true
        value = invTransferFunc(ms)
        callbackDisabled = false
        roundDurationText.text = ms + " ms"
      }
    }

    function transferFunc(val){
      var ms;
      var b = Math.pow(0.2, 1/49)
      var a = 100 * Math.pow(5, 1/49)
      if (val >= 50) {
        ms = (-0.4 * val) + 40
      } else {
        ms = a * Math.pow(b, val)
      }

      return Math.round(ms)
    }

    function invTransferFunc(ms){
      var val;
      var a = 100 * Math.pow(5, 1/49)
      if (ms <= 20) {
        val = (-2.5 * ms) + 100
      } else {
        val = (49 / Math.log(5)) * Math.log(a / ms)
      }

      return val
    }

    Text {
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
        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
          visible = false
          if (text != "") {
            executeCommand(text)
          }
          text = ""
          buttonRow.forceActiveFocus()
          event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
          visible = false
          text = ""
          buttonRow.forceActiveFocus()
          commandFieldReset()
          event.accepted = true
        } else if (event.key === Qt.Key_Up) {
          commandFieldUp()
          event.accepted = true
        } else if (event.key === Qt.Key_Down) {
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
      if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
        resultField.visible = false
        resultFieldFade.running = false
        commandField.visible = true
        commandField.forceActiveFocus()
        event.accepted = true
      } else if (event.key === Qt.Key_B && (event.modifiers & Qt.ControlModifier)) {
        ui_state++
        if (ui_state > 3) {
          ui_state = 0
        }

        if (ui_state == 0) {  // all ui elements visible
          startStopButton.visible = true
          roundButton.visible = true
          roundDurationSlider.visible = true
          roundsRow.visible = true
          numMovementsRow.visible = true
        }
        else if (ui_state == 1) {  // only start/stop and round buttons
          roundDurationSlider.visible = false
          roundsRow.visible = false
          numMovementsRow.visible = false
        }
        else if (ui_state == 2) {  // only #rounds and #movement labels
          startStopButton.visible = false
          roundButton.visible = false
          roundsRow.visible = true
          numMovementsRow.visible = true
        }
        else if (ui_state == 3) {  // none
          roundsRow.visible = false
          numMovementsRow.visible = false
        }
        event.accepted = true
      } else if (event.key === Qt.Key_E && (event.modifiers & Qt.ControlModifier)) {
        if (startStopButton.text === "start") {
            start()
        } else {
            stop()
        }
        event.accepted = true
      } else if (event.key === Qt.Key_D && (event.modifiers & Qt.ControlModifier)) {
        round()
        event.accepted = true
      } else if (event.key === Qt.Key_F && (event.modifiers & Qt.ControlModifier)) {
        vis.focusOnCenterOfMass()
        event.accepted = true
      }
    }

    A_Button {
      id: startStopButton
      text: "start"
      onClicked: {
        if (text == "start") {
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
  }
}
