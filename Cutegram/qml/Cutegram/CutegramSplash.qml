import QtQuick 2.0
import AsemanTools 1.0
import Cutegram 1.0

Rectangle {
    width: 100
    height: 62
    color: Desktop.titleBarColor

    Image {
        id: logo_img
        anchors.centerIn: parent
        width: 192
        height: width
        sourceSize: Qt.size(width,height)
        source: "files/icon.png"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logo_img.bottom
        spacing: 20*Devices.density

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: AsemanApp.globalFontFamily
            font.pixelSize: 30*Devices.fontDensity
            text: "Cutegram"
            color: Desktop.titleBarTextColor
        }

        Indicator {
            id: indicator
            anchors.horizontalCenter: parent.horizontalCenter
            indicatorSize: 22*Devices.density
            modern: true
            light: Desktop.titleBarIsDark
            Component.onCompleted: start()
        }
    }

    AsemanLogo {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8*Devices.density
        height: 30*Devices.density
        dark: !Desktop.titleBarIsDark
    }
}
