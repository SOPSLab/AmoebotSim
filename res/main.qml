import QtQuick 2.2
import QtQuick.Window 2.0
import QtQuick.Controls 1.1
import VisItem 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Hello World"

    VisItem {
        anchors.fill: parent
    }
}
