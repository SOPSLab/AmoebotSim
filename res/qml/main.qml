/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import VisItem 1.0

ApplicationWindow {
  id: appWindow
  visible: true
  color: "black"
  minimumWidth: 900
  minimumHeight: 600
  title: "AmoebotSim"

  onWidthChanged: {
    vis.width = width
  }

  onHeightChanged: {
    vis.height = height
  }

  signal algSelected(string algName)
  signal instantiate(string algName)

  signal runScript(string scriptPath)

  signal start()
  signal stop()
  signal step()
  signal exportMetrics()
  signal focusOnCenterOfMass()

  function log(msg, isError) {
    fieldLayout.forceActiveFocus()

    resultField.text = msg
    resultField.showsError = isError;
    resultField.opacity = 1
    resultField.visible = true
    resultFieldFade.running = true
  }

  function setLabelStart() {
    startStopButton.text = "Start"
  }

  function setLabelStop() {
    startStopButton.text = "Stop"
  }

  function setMetrics(metricInfo) {
    metricList.model = metricInfo
  }

  function setResolution(_width, _height) {
    if (_width < appWindow.minimumWidth) {
      appWindow.width = appWindow.minimumWidth
    } else {
      appWindow.width = _width
    }

    if (_height < appWindow.minimumHeight) {
      appWindow.height = appWindow.minimumHeight
    } else {
      appWindow.height = _height
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
    focus: true
    Keys.onPressed: {
      if (event.modifiers & Qt.ControlModifier) {
        if (event.key === Qt.Key_H) {
          sidebar.visible = !sidebar.visible
          event.accepted = true
        } else if (event.key === Qt.Key_S) {
          (startStopButton.text === "Start") ? start() : stop()
          event.accepted = true
        } else if (event.key === Qt.Key_D) {
          step()
          event.accepted = true
        } else if (event.key === Qt.Key_E) {
          exportMetrics()
          event.accepted = true
        } else if (event.key === Qt.Key_F) {
          vis.focusOnCenterOfMass()
          event.accepted = true
        }
      }
    }
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

  Item {
    id: fieldLayout
    anchors.left: vis.left
    anchors.leftMargin: 10
    anchors.bottom: vis.bottom
    anchors.bottomMargin: 10
    width: sidebar.visible ? vis.width - (sidebar.width + 30) : vis.width - 20
    height: 30

    A_ResultTextField {
      id: resultField
      visible: false
      opacity: 0
      anchors.fill: parent

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
  }

  ColumnLayout {
    id: sidebar
    spacing: 10
    anchors.right: vis.right
    anchors.top: vis.top
    anchors.bottom: vis.bottom
    anchors.margins: 10
    width: 280
    height: vis.height - 20

    ComboBox {
      id: algorithmSelectBox
      objectName: "algorithmSelectBox"
      Layout.preferredWidth: parent.width
      Layout.preferredHeight: 35

      onCurrentIndexChanged: {
        algSelected(currentText)
      }

      style: ComboBoxStyle {
        // Black for the current text is default on Windows, but not on macOS.
        textColor: "black"
      }
    }

    ScrollView {
      id: parameterView
      Layout.preferredWidth: parent.width
      Layout.preferredHeight: 105
      verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

      ListView {
        id: parameterList
        objectName: "parameterList"
        anchors.fill: parent

        model: parameterModel
        delegate: Row {
          Text {
            id: parameterText
            width: 200
            text: model.parameterName + ": "
          }

          TextField {
            width: sidebar.width - 30 - parameterText.width
            textColor: "black"
            style: TextFieldStyle {
              background: Rectangle {
                implicitHeight: 30
                border.width: 1
                border.color: "#888"
                color: "#eee"
                opacity: 0.9
              }
            }

            onEditingFinished: model.parameterValue = text
          }
        }
      }
    }

    A_Button {
      id: instantiateButton
      text: "Instantiate"
      Layout.preferredWidth: parent.width

      onClicked: {
        vis.forceActiveFocus()
        instantiate(algorithmSelectBox.currentText)
      }
    }

    A_Button {
      id: runScriptButton
      text: "Run Script"
      Layout.preferredWidth: parent.width

      onClicked: {
        runScriptFileDialog.open()
      }
    }

    FileDialog {
      id: runScriptFileDialog
      objectName: "runScriptFileDialog"
      title: "Choose script file"
      nameFilters: [ "JavaScript files (*.js)" ]
      selectedNameFilter: "JavaScript files (*.js)"

      property string executableDir: ""

      onAccepted: {
        var scriptPath = fileUrl.toString().replace("file:///" + executableDir + "/", "")
        runScript(scriptPath)
        log("Ran script: " + scriptPath, false)
      }
      onRejected: {
        log("No script selected", true)
      }
    }

    ScrollView {
      id: metricView
      Layout.preferredWidth: parent.width
      Layout.preferredHeight: 200
      verticalScrollBarPolicy: Qt.ScrollBarAsNeeded

      ListView {
        id: metricList
        anchors.fill: parent
        spacing: 5

        delegate: Row {
          Text {
            id: metricName
            width: 150
            color: "black"
            text: model.modelData[0] + ": "
          }
          Text {
            id: metricValue
            width: sidebar.width - 15 - metricName.width
            color: "black"
            text: Math.round(model.modelData[1] * 10000) / 10000
          }
        }
      }
    }

    Rectangle {
      id: fillRectangle
      Layout.preferredWidth: parent.width
      Layout.fillHeight: true
      color: "transparent"
    }

    RowLayout {
      id: stepDurationRow
      Layout.bottomMargin: 15

      Rectangle {
        Layout.preferredWidth: 130
        Text {
          anchors.left: parent.left
          text: "Step Duration:"
        }
      }

      Rectangle {
        Layout.preferredWidth: 30
        Text {
          id: stepDurationText
          anchors.left: parent.left
          text: ""
        }
      }
    }

    Slider {
      id: stepDurationSlider
      objectName: "stepDurationSlider"
      Layout.preferredWidth: parent.width

      orientation: Qt.Horizontal
      minimumValue: 1.0
      maximumValue: 100.0
      stepSize: 0.0
      updateValueWhileDragging: true
      value: 1.0

      signal stepDurationChanged(int value)
      property bool setterDisabled: false
      property bool callbackDisabled: false

      onValueChanged: {
        if (!callbackDisabled) {
          stepDurationChanged(transferFunc(value))
          stepDurationText.text = transferFunc(value) + " ms"
        }
      }

      // When changing the step duration "value" via slider, disable the
      // "setStepDuration" function because it is always called when "value"
      // changes. This is because "onValueChanged" calls "stepDurationChanged",
      // which in turn calls "setStepDuration". We break this cycle by disabling
      // the latter.
      onPressedChanged: {
        setterDisabled = !setterDisabled
      }

      // When setting the ms value via console this setter is called. This
      // setter changes the value of the slider which results in a call of
      // "onValueChanged". As explained above, this creates a call cycle that we
      // break by disabling the callback "onValueChanged" for this value change.
      function setStepDuration(ms) {
        if (!setterDisabled) {
          callbackDisabled = true
          value = invTransferFunc(ms)
          callbackDisabled = false
          stepDurationText.text = ms + " ms"
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
    }

    RowLayout {
      id: controlButtonRow
      spacing: 5
      Layout.preferredWidth: parent.width

      A_Button {
        id: startStopButton
        implicitWidth: 80
        text: "Start"
        onClicked: (text == "Start") ? start() : stop()
      }

      A_Button {
        id: stepButton
        implicitWidth: 80
        text: "Step"
        onClicked: step()
      }

      A_Button {
        id: metricsButton
        implicitWidth: 80
        text: "Metrics"
        onClicked: exportMetrics()
      }
    }
  }
}
