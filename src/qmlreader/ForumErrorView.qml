import QtQuick 2.0
import QtQuick.Controls 2.0

ListView {
    model: currentSubscription ? currentSubscription.errors : undefined
    delegate: Label {
        text: modelData.title
    }
}
