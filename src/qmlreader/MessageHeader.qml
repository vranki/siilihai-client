import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    Layout.fillWidth: true
    height: infoFrame.height
    Frame {
        id: infoFrame
        width: parent.width

        background: Rectangle {
            color: backgroundColor
            border.color: "black"
            radius: 4
        }
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            Label { text: "From" }
            Label {
                text: currentMessage ? currentMessage.author : ""
                Layout.fillWidth: true
                font.bold: true
            }
            Label { text: "Subject" }
            Label {
                text: currentMessage ? currentMessage.displayName : ""
                Layout.fillWidth: true
                font.bold: true
            }
            Label { text: "Date" }
            Label {
                text: currentMessage ? currentMessage.lastChange : ""
                Layout.fillWidth: true
                font.bold: true
            }
        }
    }
    ToolBar {
        visible: infoFrame.visible
        anchors {
            right: parent.right
            rightMargin: 10
            top: parent.top
            topMargin: 5
        }
        ToolButton {
            contentItem: RowLayout {
                Image {
                    source: "qrc:/data/emblem-web.png"
                    anchors.verticalCenter: parent.verticalCenter
                    Layout.preferredWidth: Layout.preferredHeight
                    Layout.preferredHeight: parent.height
                }
                Label {
                    text: "View in browser"
                    // When we have Qt 5.10:
                    // icon.source: "qrc:/data/emblem-web.png"
                    enabled: (currentMessage != undefined) && currentMessage.url
                }
            }
            onClicked: Qt.openUrlExternally(currentMessage.url)
        }
    }
}
