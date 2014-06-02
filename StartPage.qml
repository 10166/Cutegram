import QtQuick 2.0
import org.sialan.telegram 1.0

Rectangle {
    id: start_page
    width: 100
    height: 62

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        hoverEnabled: true
        onWheel: wheel.accepted = true
    }

    Authenticating {
        id: auth
        anchors.fill: parent
    }

    Column {
        id: connecting_frame
        width: connecting_text.width
        spacing: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        anchors.horizontalCenter: parent.horizontalCenter
        visible: Telegram.waitAndGet == Enums.NoWaitAndGet || finished_timer.running

        onVisibleChanged: {
            if( visible )
                indict.start()
            else
                indict.stop()
        }

        Indicator {
            id: indict
            width: parent.width
            height: indicatorSize
            indicatorSize: 32
            source: "files/indicator_light.png"
            Component.onCompleted: start()
        }

        Text {
            id: connecting_text
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Connecting...")
            font.family: globalNormalFontFamily
            font.pointSize: 11
            color: "#ffffff"
        }
    }

    SialanSplash {
        id: splash
        anchors.fill: parent
        Component.onCompleted: start()
    }

    Timer {
        id: finished_timer
        interval: 4000
        onTriggered: auth.start()
    }

    Component.onCompleted: finished_timer.start()
}
