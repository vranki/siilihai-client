import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    MessageView {
        anchors.fill: parent
        visible: currentMessage
    }
    ForumErrorView {
        anchors.fill: parent
        visible: currentSubscription && !currentMessage
    }
}
