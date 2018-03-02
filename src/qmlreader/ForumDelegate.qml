import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

ListView {
    id: forumDelegate
    width: parent.width
    height: contentHeight
    model: forumDelegate.subscription === currentSubscription ? modelData.subscribedGroups : undefined
    property var subscription: modelData
    readonly property bool unread: modelData.unreadCount > 0

    header: ItemDelegate {
        id: forumButton
        width: parent.width
        height: 20
        onClicked: currentSubscription = modelData
        background: Rectangle {
            color: highlightColor
            visible: currentSubscription === subscription
        }
        RowLayout {
            width: parent.width
            Image {
                source: modelData.faviconUrl
                fillMode: Image.PreserveAspectCrop
                Layout.preferredWidth: Layout.preferredHeight
                Layout.preferredHeight: parent.height
            }
            Label {
                text: modelData.alias
                font.bold: unread
                Layout.fillWidth: true
                clip: true
            }
            Label {
                text: modelData.unreadCount
                font.bold: unread
            }
        }
    }
    delegate: ItemDelegate {
        width: parent.width
        highlighted: currentGroup === modelData
        readonly property bool unread: modelData.unreadCount > 0
        font.bold: unread
        onClicked: {
            currentSubscription = forumDelegate.subscription
            currentGroup = modelData
        }
        background: Rectangle {
            color: highlightColor
            visible: highlighted
        }
        RowLayout {
            anchors.fill: parent
            Image {
                source: modelData.unreadCount ? "qrc:/data/folder-new.png" : "qrc:/data/folder.png"
                Layout.preferredWidth: Layout.preferredHeight
                Layout.preferredHeight: parent.height * 0.75
                fillMode: Image.PreserveAspectFit
            }
            Label {
                text: modelData.displayName
                Layout.fillWidth: true
                font.bold: unread
            }
            Label {
                text: modelData.unreadCount
                font.bold: unread
                Rectangle {
                    anchors.fill: parent
                    color: "white"
                    z: -1
                }
            }
        }
    }
}
