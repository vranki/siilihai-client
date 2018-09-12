import QtQuick 2.0
import QtQuick.Controls 2.0
import QtWebEngine 1.5
import QtQuick.Layouts 1.3

ColumnLayout {
    property string body: currentMessage ? currentMessage.body : ""
    property string blankUrl: "qrc:/data/blankmessage/index.html"
    MessageHeader {}

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

    focus: true //currentMessage ? true : false
    Keys.onSpacePressed: {
        console.log("SPACE")
        event.accepted = true;
        siilihaiqml.moveToNextMessage()
    }
}
