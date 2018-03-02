import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

Item {
    property var message: modelData
    readonly property bool isThread: !modelData.ordernum
    readonly property bool shown: isThread || maximized
    readonly property bool unread: isThread ? thread.unreadCount : !modelData.isRead

    width: parent.width
    visible: shown
    height: shown ? 20 : 0

    function select() {
        currentMessage = null
        currentMessage = modelData
        threadDelegate.currentIndex = index
    }

    Rectangle {
        color: highlightColor
        visible: currentMessage === modelData
        anchors.fill: parent
    }
    MouseArea {
        anchors.fill: parent
        onClicked: select()
    }
    MouseArea {
        id: leftControls
        width: parent.width * 0.03
        height: parent.height
        onClicked: maximized = !maximized
        Text {
            text: isThread && (thread.count > 1) ? "⯈" : ""
            rotation: maximized ? 90 : 0
            anchors.centerIn: parent
            font.bold: unread
        }
    }
    RowLayout {
        id: rowLayout
        anchors {
            left: leftControls.right
            leftMargin: isThread ? 0 : 20
            right: parent.right
            rightMargin: 20
            top: parent.top
            bottom: parent.bottom
        }
        Image {
            source: sourceImage()
            Layout.preferredWidth: Layout.preferredHeight
            Layout.preferredHeight: parent.height
            fillMode: Image.PreserveAspectCrop
            function sourceImage() {
                if(isThread) {
                    return unread ? "qrc:/data/folder-new.png" : "qrc:/data/folder.png"
                } else {
                    return unread ? "qrc:/data/mail-unread.png" : "qrc:/data/emblem-mail.png"
                }
            }
        }
        Label {
            font.bold: unread
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: modelData.displayName
        }
        Label {
            font.bold: unread
            text: modelData.lastChange
        }
        Label {
            font.bold: unread
            text: unreadCountText()
            width: parent.width * 0.3
            function unreadCountText() {
                if(isThread && thread.unreadCount) {
                    if(thread.hasMoreMessages) {
                        return thread.unreadCount + "+"
                    } else {
                        return thread.unreadCount
                    }
                }
                return ""
            }
        }
    }
}
