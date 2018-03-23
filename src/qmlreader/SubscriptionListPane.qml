import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

Item {
    Layout.fillHeight: true
    ListView {
        anchors.top: parent.top
        anchors.bottom: subscriptionControls.top
        width: parent.width
        model: siilihai.forumDatabase.subscriptions
        clip: true
        delegate: ForumDelegate {
            width: parent.width
        }
        ScrollBar.vertical: ScrollBar {}
    }
    Rectangle {
        id: subscriptionControls
        anchors.bottom: parent.bottom
        color: "black"
        width: parent.width
        height: width / 4
        Text {
            width: parent.width/2
            text: qsTr("⊖")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"
            font.pixelSize: parent.height * 0.8
        }
        Text {
            anchors.right: parent.right
            width: parent.width/2
            text: qsTr("⊕")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"
            font.pixelSize: parent.height * 0.8
            MouseArea {
                anchors.fill: parent
                onClicked: siilihai.showSubscribeForumDialog()
            }
        }
    }
}
