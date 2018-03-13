import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2

Window {
    width: 400
    height: 600
    title: "Siilihai errors"
    visible: siilihai.errorMessages.length
    ListView {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: dismissButton.top
        }

        model: siilihai.errorMessages
        delegate: ColumnLayout {
            Layout.fillWidth: true
            Label {
                text: modelData
            }
            Rectangle {
                color: "black"
                width: parent.width / 4
                height: 1
            }
        }
    }
    Button {
        id: dismissButton
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "Dismiss"
        onClicked: siilihai.dismissMessages()
    }
    onClosing: siilihai.dismissMessages()
}
