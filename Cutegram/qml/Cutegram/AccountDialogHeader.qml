import QtQuick 2.0
import AsemanTools 1.0
import Cutegram 1.0
import CutegramTypes 1.0

Rectangle {
    id: header
    height: 60*Devices.density
    color: backColor2

    property Dialog currentDialog

    property bool isChat: currentDialog? currentDialog.peer.chatId != 0 : false
    property User user: telegramObject.user(currentDialog.peer.userId)
    property Chat chat: telegramObject.chat(currentDialog.peer.chatId)

    property bool refreshing: false

    property real typeUserStatusOffline: 0x8c703f
    property real typeUserStatusEmpty: 0x9d05049
    property real typeUserStatusOnline: 0xedb93949

    signal clicked()

    onRefreshingChanged: {
        if( refreshing )
            indicator.start()
        else
            indicator.stop()
    }

    Text {
        id: title_txt
        anchors.centerIn: parent
        anchors.horizontalCenter: parent.horizontalCenter
        color: textColor0
        font.pixelSize: 15*Devices.fontDensity
        font.family: AsemanApp.globalFontFamily
        text: {
            if( !currentDialog )
                return ""
            if( isChat )
                return chat? chat.title : ""
            else
                return user? user.firstName + " " + user.lastName : ""
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: title_txt.horizontalCenter
        anchors.bottomMargin: -4*Devices.density
        color: masterPalette.highlight
        font.pixelSize: 9*Devices.fontDensity
        font.family: AsemanApp.globalFontFamily
        visible: currentDialog != telegramObject.nullDialog
        text: {
            var result = ""
            var list = currentDialog.typingUsers
            if( list.length == 0 ) {
                if( isChat ) {
                    result += chat.participantsCount + qsTr(" participants")
                } else {
                    var isOnline = header.user.status.classType == typeUserStatusOnline
                    result += isOnline? qsTr("Online") : Cutegram.getTimeString(CalendarConv.fromTime_t(header.user.status.wasOnline)) + qsTr(" was online")
                }

                return result
            }

            for( var i=0; i<list.length; i++ ) {
                var uid = list[i]
                var user = telegramObject.user(list[i])
                var uname = user.firstName + " " + user.lastName
                result += uname.trim()

                if( i < list.length-1 )
                    result += ", "
            }

            result = result.trim() + " typing..."
            return result
        }
    }

    Indicator {
        id: indicator
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: y
        light: false
        modern: true
        indicatorSize: 20*Devices.density
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: header.clicked()
    }
}
