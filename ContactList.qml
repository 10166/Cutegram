import QtQuick 2.0

Rectangle {
    id: contact_list
    width: 100
    height: 62
    color: "#ffffff"

    property int current

    Connections {
        target: Telegram
        onContactsChanged: {
            privates.contacts_refreshed = true
            clist.refresh()
        }
        onDialogsChanged: {
            privates.dialogs_refreshed = true
            clist.refresh()
        }
        onStartedChanged: {
            Telegram.updateDialogList()
            Telegram.updateContactList()
        }
        onIncomingMsg: {
            Telegram.updateDialogListUsingTimer()
        }
        onUserStatusChanged: {
            clist.refresh()
        }
        onMsgChanged: {
            Telegram.updateDialogListUsingTimer()
        }
    }

    QtObject {
        id: privates
        property bool contacts_refreshed: false
        property bool dialogs_refreshed: false
    }

    Indicator {
        id: indicator
        anchors.fill: parent
        source: "files/indicator.png"
        Component.onCompleted: start()
    }

    ContactListHeader {
        id: header
        height: 53
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        onSelected: {
            cnct_change_timer.uid = uid
            cnct_change_timer.restart()
            menu.stop()
        }

        Timer {
            id: cnct_change_timer
            interval: 400
            repeat: false
            onTriggered: {
                contact_list.current = uid
                chatFrame.chatView.progressIndicator.stop()
            }
            property int uid
        }
    }

    ListView {
        id: clist
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        maximumFlickVelocity: 2500
        flickDeceleration: 2500
        clip: true
        model: ListModel{}
        delegate: ContactListItem {
            id: item
            height: 57
            width: clist.width
            uid: user_id
            realId: item.isDialog? dialog_id : user_id
            selected: realId == contact_list.current
            onClicked: contact_list.current = item.isDialog? dialog_id : item.uid
        }

        function refresh() {
            if( !privates.contacts_refreshed || !privates.dialogs_refreshed )
                return

            indicator.stop()
            model.clear()
            var contacts = Telegram.contactListUsers()
            var dialogs = Telegram.dialogListIds()

            for( var i=0; i<dialogs.length; i++ ) {
                model.append( {"user_id": 0, "dialog_id": dialogs[i]} )
                if( Telegram.dialogIsChat(dialogs[i]) )
                    Telegram.loadChatInfo(dialogs[i])
                else
                    Telegram.loadUserInfo(dialogs[i])
                var cIndex = contacts.indexOf(dialogs[i])
                if( cIndex != -1 )
                    contacts.splice(cIndex,1)
            }
//            for( var i=0; i<contacts.length; i++ ) {
//                model.append( {"user_id":contacts[i], "dialog_id": 0} )
//                Telegram.loadUserInfo(contacts[i])
//            }
        }

        Component.onCompleted: refresh()
    }

    PhysicalScrollBar {
        scrollArea: clist; height: clist.height; width: 8
        anchors.right: clist.right; anchors.top: clist.top; color: "#333333"
    }
}
