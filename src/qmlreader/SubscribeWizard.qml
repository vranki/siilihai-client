import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4

Dialog {
    visible: false
    title: qsTr("Subscribe")
    width: 400
    height: 400
    onVisibleChanged: {
        if(visible) {
            siilihai.subscriptionManagement.listForums()
        } else {
            siilihai.subscriptionManagement.resetForumList()
        }
    }

    Rectangle {
        id: header
        color: "white"
        width: parent.width
        height: 30
        Text {
            color: "Black"
            font.pixelSize: 22
            text: "Subscribe to a forum"
        }
    }
    ScrollView {
        width: parent.width
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        ListView {
            id: forumListView
            model: siilihai.subscriptionManagement.forumList

            delegate: Rectangle {
                width: parent.width
                height: itemText.height
                color: forumListView.currentIndex === index ? highlightColor : "white"
                Text {
                    id: itemText
                    text: modelData.alias
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: forumListView.currentIndex = index
                }
            }
        }
    }

    standardButtons: StandardButton.Cancel | StandardButton.next
}
