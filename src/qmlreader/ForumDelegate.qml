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
    property int lastSelectedGroupIndex: 0

    header: Button {
        id: forumButton
        width: parent.width
        height: 30
        onClicked: {
            currentMessage = null
            currentGroup = null
            currentSubscription = modelData
            if(currentSubscription.subscribedGroups.length > lastSelectedGroupIndex) {
                currentGroup = currentSubscription.subscribedGroups[lastSelectedGroupIndex]
            }
        }
        highlighted: currentSubscription === subscription

        RowLayout {
            anchors.fill: parent
            Item {
                Layout.preferredWidth: Layout.preferredHeight
                Layout.preferredHeight: parent.height

                Image {
                    source: "qrc:/data/emblem-web.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    visible: favIcon.status !== Image.Ready
                }
                Image {
                    id: favIcon
                    source: modelData.faviconUrl
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                }
                BusyIndicator {
                    anchors.fill: parent
                    running: modelData.beingUpdated || modelData.scheduledForUpdate
                }
                Image {
                    source: "qrc:/data/dialog-error.png"
                    fillMode: Image.PreserveAspectFit
                    anchors.fill: parent
                    visible: modelData.errors.length
                }
            }
            Label {
                text: modelData.alias
                font.bold: unread
                color: highlighted ? "white" : "black"
                Layout.fillWidth: true
                clip: true
            }
            Label {
                text: modelData.unreadCount
                font.bold: unread
                color: highlighted ? "white" : "black"
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
            lastSelectedGroupIndex = index
        }
        background: Rectangle {
            color: highlightColor
            visible: highlighted
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
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
                    color: highlighted ? highlightColor : "white"
                    z: -1
                }
            }
        }
    }
}
