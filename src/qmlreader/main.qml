import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import com.siilihai.siilihai 1.0

ApplicationWindow {
    id: siilihaiqml
    visible: true
    width: 1024
    height: 768
    title: qsTr("Siilihai")
    property var currentSubscription: null
    property var currentGroup: null
    property var currentMessage: null
    property bool quitReally: false
    readonly property color highlightColor: "#bebeff"
    readonly property color backgroundColor: "#f0f0f0"
    readonly property int lineHeight: 20
    signal moveToNextMessage

    SiilihaiClient {
        id: siilihai
        onCloseUi: {
            quitReally = true
            close()
        }
    }
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "Update"
                onClicked: siilihai.updateClicked()
                enabled: siilihai.state === SiilihaiClient.SH_READY
            }
            ToolButton {
                text: "Cancel"
                onClicked: siilihai.cancelClicked()
                enabled: siilihai.state === SiilihaiClient.SH_READY
            }
            Item { Layout.fillWidth: true }
            CheckBox {
                Layout.alignment: Qt.AlignRight
                checked: siilihai.offline
                onClicked: siilihai.setOffline(!checked)
                text: "Offline"
            }
        }
    }
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label { text: siilihai.statusMessage }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        SubscriptionListPane {
            width: 200
            height: parent.height
        }
        SplitView {
            orientation: Qt.Vertical
            ThreadListView {
                height: 200
            }
            ContentView { }
        }
    }

    ErrorMessages {}

    SubscribeWizard {
        id: subscribeWizard
    }
    Component.onCompleted: {
        siilihai.launchSiilihai()
    }

    onClosing: {
        if(!quitReally) {
            close.accepted = false
            siilihai.haltSiilihai()
        }
        quitReally = true
    }


    Connections {
        target: siilihai
        onShowSubscribeForumDialog: subscribeWizard.open()
    }
}
