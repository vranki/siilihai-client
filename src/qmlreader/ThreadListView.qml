import QtQuick 2.0
import QtQuick.Controls 2.1

ListView {
    width: parent.width
    model: currentGroup ? currentGroup.threads : undefined
    clip: true
    interactive: false
    delegate: ThreadDelegate { }
    ScrollBar.vertical: ScrollBar {}
    MouseArea {
        acceptedButtons: Qt.NoButton
        preventStealing: true
        propagateComposedEvents: true
        anchors.fill: parent
        onWheel: {
            parent.contentY -= (wheel.angleDelta.y / 120) * lineHeight
            parent.returnToBounds()
        }
    }
}
