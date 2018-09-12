import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1
import com.siilihai.siilihai 1.0

ListView {
    id: threadDelegate
    width: parent.width - 20
    height: contentHeight
    model: modelData.messages
    property var thread: modelData
    property bool maximized: false
    delegate: MessageButton {}
    interactive: false
    anchors.right: parent.right

    footer: Button {
        width: parent.width * 0.5
        height: visible ? lineHeight : 0
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Show more messages.."
        visible: maximized && siilihai.state === SiilihaiClient.SH_READY && modelData.hasMoreMessages
        onClicked: siilihai.getMoreMessages(thread)
    }
    Connections {
        target: siilihaiqml
        onSelectMessage: {
            if(message.thread === thread) {
                maximized = true
            }
        }
    }
}
