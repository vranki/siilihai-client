import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    width: parent.width

    MessageView {
        anchors.fill: parent
        visible: currentMessage
    }
    ForumErrorView {
        id: forumErrorView
        anchors.fill: parent
        visible: currentSubscription && !currentMessage
    }
    Column {
        visible: !forumErrorView.visible && !currentMessage
        anchors.fill: parent
        Image {
            source: "qrc:///data/blankmessage/siilis3.png"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "www.siilihai.com"
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "Powered by TapaTalk"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
