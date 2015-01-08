import QtQuick 2.0
import AsemanTools 1.0
import Cutegram 1.0
import CutegramTypes 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: acc_view
    width: 100
    height: 62
    color: "#222222"

    property alias telegramObject: dialogs.telegramObject
    property color framesColor: "#aaffffff"
    property alias currentDialog: dialogs.currentDialog

    AccountDialogList {
        id: dialogs
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        clip: true
    }

    Item {
        id: properties_frame
        anchors.left: dialogs.right
        anchors.right: parent.right
        height: parent.height
        clip: true
    }

    Item {
        id: message_box
        height: parent.height
        y: header.properties && header.properties.inited? header.properties.height : 0
        anchors.left: dialogs.right
        anchors.right: parent.right
        clip: true

        Behavior on y {
            NumberAnimation{ easing.type: Easing.OutCubic; duration: 400 }
        }

        AccountMessageList {
            id: messages
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            topMargin: header.height
            bottomMargin: send_msg.height
            telegramObject: dialogs.telegramObject
            currentDialog: dialogs.currentDialog
            onForwardRequest: forward_component.createObject(message_box, {"forwardMessage":message})
        }

        Item {
            id: header_blur
            anchors.fill: header
            clip: true

            FastBlur {
                anchors.top: parent.top
                width: messages.width
                height: messages.height
                source: messages
                radius: 64
            }
        }

        Item {
            id: send_msg_blur
            anchors.fill: send_msg
            clip: true

            FastBlur {
                anchors.bottom: parent.bottom
                width: messages.width
                height: messages.height
                source: messages
                radius: 64
            }
        }

        AccountDialogHeader {
            id: header
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            color: currentDialog.encrypted? "#aa000000" : framesColor
            currentDialog: dialogs.currentDialog
            refreshing: messages.refreshing
            onClicked: {
                if( properties )
                    properties.end()
                else
                if( currentDialog != telegramObject.nullDialog )
                    properties = user_prp_component.createObject(properties_frame)
            }

            property UserProperties properties
        }

        AccountSendMessage {
            id: send_msg
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: framesColor
            currentDialog: dialogs.currentDialog
            onAccepted: messages.sendMessage(text)
            onEmojiRequest: {
                var item = emoticons_component.createObject(message_box)
                var w = 260*Devices.density
                var h = w

                pointerDialog.pointerLeftMargin = w*0.6
                main.showPointDialog(item, x-w*0.6-20*Devices.density, y, w, h)
            }
        }

        Item {
            id: drop_file_blur
            anchors.fill: drop_file
            clip: true
            opacity: drop_file.visibleRatio
            visible: opacity != 0

            FastBlur {
                anchors.top: parent.top
                anchors.topMargin: -messages.topMargin
                width: messages.width
                height: messages.height
                source: messages
                radius: 32
            }
        }

        DialogDropFile {
            id: drop_file
            anchors.left: parent.left
            anchors.top: header.bottom
            anchors.right: parent.right
            anchors.bottom: send_msg.top
            currentDialog: dialogs.currentDialog
        }

        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            opacity: header.properties && header.properties.inited? 0.5 : 0
            visible: header.properties? true : false

            Behavior on opacity {
                NumberAnimation{ easing.type: Easing.OutCubic; duration: 400 }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton | Qt.LeftButton
                hoverEnabled: true
                onWheel: wheel.accepted = true
                onClicked: BackHandler.back()
            }
        }
    }

    Component {
        id: emoticons_component
        Emoticons {
            id: emoticons
            anchors.right: parent.right
            anchors.bottom: send_msg.top
            anchors.top: header.bottom
            width: 200*Devices.density
            onSelected: send_msg.insertText(code)
        }
    }

    Component {
        id: user_prp_component
        UserProperties {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            currentDialog: acc_view.currentDialog
            color: Desktop.titleBarColor
        }
    }

    Component {
        id: forward_component
        Item {
            id: forward_item
            anchors.fill: messages
            clip: true

            property alias forwardMessage: forward_page.forwardMessage
            property alias forwardDialog: forward_page.forwardDialog

            FastBlur {
                anchors.fill: parent
                source: messages
                radius: 64
            }

            ForwardPage {
                id: forward_page
                anchors.fill: parent
                onCloseRequest: forward_item.destroy()
            }

            Connections {
                target: acc_view
                onCurrentDialogChanged: forwardDialog = acc_view.currentDialog
            }
        }
    }
}
