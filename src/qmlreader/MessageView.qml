import QtQuick 2.0
import QtQuick.Controls 2.0
import QtWebEngine 1.5
import QtQuick.Layouts 1.3

ColumnLayout {
    property string body: currentMessage ? currentMessage.body : ""
    property string blankUrl: "qrc:/data/blankmessage/index.html"
    Frame {
        id: infoFrame
        Layout.fillWidth: true
        background: Rectangle {
            color: backgroundColor
            border.color: "black"
            radius: 4
        }
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            Label { text: "From" }
            Label {
                text: currentMessage ? currentMessage.author : ""
                Layout.fillWidth: true
                font.bold: true
            }
            Label { text: "Subject" }
            Label {
                text: currentMessage ? currentMessage.displayName : ""
                Layout.fillWidth: true
                font.bold: true
            }
            Label { text: "Date" }
            Label {
                text: currentMessage ? currentMessage.lastChange : ""
                Layout.fillWidth: true
                font.bold: true
            }
        }
    }
    ToolBar {
        visible: infoFrame.visible
        anchors {
            right: infoFrame.right
            rightMargin: 10
            top: infoFrame.top
            topMargin: 5
        }
        ToolButton {
            contentItem: RowLayout {
                Image {
                    source: "qrc:/data/emblem-web.png"
                    anchors.verticalCenter: parent.verticalCenter
                    Layout.preferredWidth: Layout.preferredHeight
                    Layout.preferredHeight: parent.height
                }
                Label {
                    text: "View in browser"
                    // When we have Qt 5.10:
                    // icon.source: "qrc:/data/emblem-web.png"
                    enabled: (currentMessage != undefined) && currentMessage.url
                }
            }
            onClicked: Qt.openUrlExternally(currentMessage.url)
        }
    }
    WebEngineView {
        id: webEngineView
        url: blankUrl
        Layout.fillWidth: true
        Layout.fillHeight: true
        onNavigationRequested: {
            request.action = WebEngineNavigationRequest.IgnoreRequest
            if(request.navigationType === WebEngineNavigationRequest.LinkClickedNavigation && request.isMainFrame)
                Qt.openUrlExternally(request.url)
        }
        onNewViewRequested: {
            request.action = WebEngineNavigationRequest.IgnoreRequest
            if(request.userInitiated) {
                Qt.openUrlExternally(request.requestedUrl)
            }
        }
        settings.javascriptEnabled: false
        settings.localContentCanAccessFileUrls: false
    }
    onBodyChanged: {
        if(body !== "") {
            webEngineView.loadHtml(body)
            currentMessage.isRead = true
        } else webEngineView.url = blankUrl
    }
    focus: currentMessage ? true : false
    Keys.onSpacePressed: {
        event.accepted = true;
        moveToNextMessage()
    }
}
