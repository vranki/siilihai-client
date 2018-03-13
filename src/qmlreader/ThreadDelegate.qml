import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1
import com.siilihai.siilihai 1.0

ListView {
    id: threadDelegate
    width: parent.width
    height: contentHeight
    model: modelData.messages
    property var thread: modelData
    property bool maximized: false
    delegate: MessageButton {}
    Connections {
        target: siilihaiqml
        onMoveToNextMessage: {
            if(!currentMessage || currentMessage.thread !== thread) return
            while(currentItem.message.isRead && currentIndex < count - 1)
                currentIndex++
            maximized = true
            currentItem.select()
            threadDelegate.positionViewAtIndex(currentIndex, ListView.Center)
        }
    }
    footer: Button {
        width: parent.width * 0.5
        height: visible ? lineHeight : 0
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Show more messages.."
        visible: maximized && siilihai.state === SiilihaiClient.SH_READY && modelData.hasMoreMessages
        onClicked: siilihai.getMoreMessages(thread)
    }
}
