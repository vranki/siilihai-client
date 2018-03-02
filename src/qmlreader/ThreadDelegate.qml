import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

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
            currentItem.select()
            positionViewAtIndex(currentIndex, ListView.Center)
        }
    }
}
