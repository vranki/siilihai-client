import QtQuick 2.0
import QtQuick.Controls 2.2

ListView {
    model: currentGroup ? currentGroup.threads : undefined
    clip: true
    interactive: false
    delegate: ThreadDelegate { }
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn
    }
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
    Connections {
        target: siilihaiqml
        onMoveToNextMessage: {
            if(!currentMessage) return
            if(!model) return
            var found = false;
            for(var i=0;i<model.length;i++) {
                var thr = model[i]
                if(found || thr === currentMessage.thread) {
                    for(var j=0;j<thr.messages.length;j++) {
                        var msg = thr.messages[j]
                        console.log(msg.name, msg.isRead)
                        if(currentMessage === msg) {
                            found = true
                        }
                        if(found && !msg.isRead) {
                            siilihaiqml.selectMessage(msg)
                            return
                        }
                    }
                }
            }
        }
    }
}
