import QtQuick 2.0
import QtGraphicalEffects 1.0
import org.sialan.telegram 1.0

Rectangle {
    id: chat_view
    width: 100
    height: 62
    clip: true

    property int current: 0
    property int cache

    property real blurRadius: 64

    property alias progressIndicator: indicator
    property bool userConfig: false
    property alias smilies: send_frame.smilies

    onCurrentChanged: {
        indicator.start()
        chat_list.clear()

        userConfig = false
        if( current == 0 )
            return

        chat_list.disableAnims = true

        var ids = Telegram.messagesOf(current)
        for( var i=0; i<ids.length; i++ ) {
            cache = ids[i]
            chat_list.append(cache)
        }

        Telegram.getHistory(current,100)
        chat_list.positionViewAtEnd()
    }

    Connections {
        target: Telegram
        onUserStatusChanged: {
        }
        onIncomingMsg: {
            indicator.stop()
            dis_anim_timer.restart()
        }
        onMessageDeleted: {
            var tmp = main.current
            main.current = 0
            main.current = tmp
        }
        onMessageRestored: {
            var tmp = main.current
            main.current = 0
            main.current = tmp
        }
        onIncomingNewMsg: {
            if( chat_view.current == 0 )
                return
            if( Telegram.messageIsDeleted(msg_id) )
                return

            var to_id = Telegram.messageToId(msg_id)
            var fr_id = Telegram.messageFromId(msg_id)
            var is_chat = Telegram.dialogIsChat(chat_view.current)
            var msg_out = Telegram.messageOut(msg_id)

            if( to_id == 0 || fr_id == 0 )
                return

            if( is_chat && to_id !== chat_view.current ){
                sendNotify(msg_id)
                return
            }
            else
            if( !is_chat ) {
                if( msg_out && to_id !== chat_view.current ){
                    sendNotify(msg_id)
                    return
                }
                else
                if( !msg_out && fr_id !== chat_view.current ){
                    sendNotify(msg_id)
                    return
                }
                else
                if( Telegram.dialogIsChat(to_id) || Telegram.dialogIsChat(fr_id) ){
                    sendNotify(msg_id)
                    return
                }
            }

            indicator.stop()
            chat_list.append(msg_id)
            if( !main.active || !main.visible ) {
                sendNotify(msg_id)
                return
            }
            if( Telegram.messageUnread(msg_id) === 1 )
                Telegram.markRead(chat_view.current)
        }
        onMsgSent: {
            var index = -1
            if( old_id == 0 )
                return

            for( var i=0; i<chat_list.model.count; i++ )
                if( chat_list.model.get(i).msg == old_id )
                {
                    index = i
                    break;
                }
            if( index == -1 )
                return

            chat_list.disableAnims = true
            dis_anim_timer.restart()

            chat_list.model.remove(index,1)
            chat_list.append(msg_id)
        }
    }

    Item {
        id: chat_list_frame
        anchors.fill: parent

        Image {
            id: image_back
            anchors.fill: parent
            sourceSize: Qt.size(width,height)
            fillMode: Image.PreserveAspectCrop
            smooth: true
            source: Gui.background.length != 0? "file://" + Gui.background : ""
        }

        ListView {
            id: chat_list
            anchors.fill: parent
            model: ListModel{}
            spacing: 5
            cacheBuffer: 500

            property bool disableAnims: false

            NumberAnimation {
                id: anim
                target: chat_list
                property: "contentY"
                easing.type: Easing.OutCubic
                duration: 400
                onRunningChanged: if(!running) chat_list.returnToBounds()
            }

            Timer {
                id: dis_anim_timer
                interval: 500
                repeat: false
                onTriggered: chat_list.disableAnims = false
            }

            header: Item {
                width: chat_list.width
                height: title_bar.height
            }

            footer: Item {
                width: chat_list.width
                height: send_frame.height + (smilies_frame.visible? smilies_frame.height : 0)
            }

            delegate: Item {
                id: item
                width: chat_list.width
                height: msg_item.height

                property variant msgItem

                Component.onCompleted: {
                    if( Telegram.messageService(msg) != 0 )
                        msgItem = msg_act_component.createObject(item, {"msg_id":msg} )
                    else
                        msgItem = msg_component.createObject(item, {"msg_id":msg} )

                    msgItem.heightChanged.connect(item.refreshHeight)
                    item.height = msgItem.height
                }

                function refreshHeight() {
                    item.height = msgItem.height
                }
            }

            function gotoIndex(idx) {
                anim.stop()
                var pos = chat_list.contentY;
                chat_list.positionViewAtIndex(idx, ListView.End);
                var destPos = chat_list.contentY;
                anim.from = pos;
                anim.to = destPos;
                anim.start()
            }

            function gotoEnd() {
                anim.stop()
                var pos = chat_list.contentY;
                chat_list.positionViewAtEnd()
                var destPos = chat_list.contentY;
                anim.from = pos;
                anim.to = destPos;
                anim.start()
            }

            function append( msg_id ) {
                var index = 0
                for( var i=0; i<model.count; i++ ) {
                    if( model.get(i).msg <= msg_id )
                        index = i+1
                    else
                        break;
                }

                if( msg_id < 0 )
                    index = model.count

                var to_end = chat_list.atYEnd

                model.insert(index, {"msg":msg_id} )

                if( to_end ) {
                    if( msg_id < 0 )
                        chat_list.gotoEnd()
                    else
                        chat_list.positionViewAtEnd()
                }
            }

            function clear() {
                model.clear()
            }
        }
    }

    PhysicalScrollBar {
        scrollArea: chat_list; width: 8
        anchors.right: parent.right; anchors.top: title_bar.bottom;
        anchors.bottom: smilies_frame.top; color: "#ffffff"
    }

    Item {
        id: footer_blur_frame
        anchors.fill: send_frame
        clip: true

        Mirror {
            width: chat_view.width
            height: chat_view.height
            anchors.bottom: parent.bottom
            source: chat_list_frame
        }
    }

    Item {
        id: header_blur_frame
        anchors.fill: title_bar
        clip: true

        Mirror {
            width: chat_view.width
            height: chat_view.height
            anchors.top: parent.top
            source: chat_list_frame
        }
    }

    Item {
        id: down_blur_frame
        anchors.fill: down_button
        clip: true
        opacity: down_button.opacity

        Mirror {
            width: chat_view.width
            height: chat_view.height
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -send_frame.height
            source: chat_list_frame
        }
    }

    ChatTitleBar {
        id: title_bar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        current: chat_view.current
        onClicked: showConfigure(chat_view.current)

        Indicator {
            id: indicator
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: height
        }
    }

    Button {
        id: down_button
        anchors.bottom: send_frame.top
        anchors.left: send_frame.left
        anchors.right: send_frame.right
        height: 30
        normalColor: imageBack? "#55ffffff" : "#77ffffff"
        highlightColor: "#994098BF"
        icon: "files/down.png"
        iconHeight: 18
        opacity: chat_list.atYEnd || anim.running? 0 : 1
        visible: opacity != 0
        onClicked: chat_list.positionViewAtEnd()

        Behavior on opacity {
            NumberAnimation{ easing.type: Easing.OutCubic; duration: 400 }
        }
    }

    Item {
        id: smilies_blur_frame
        anchors.fill: smilies_frame
        clip: true
        opacity: smilies_frame.opacity

        Mirror {
            width: chat_view.width
            height: chat_view.height
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -send_frame.height
            source: chat_list_frame
        }
    }

    Smilies {
        id: smilies_frame
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: send_frame.top
        height: chat_view.smilies? 100 : 0
        visible: height > 0
        onSelected: {
            send_frame.textInput.insert( send_frame.textInput.cursorPosition, code )
        }

        onHeightChanged: {
            if( height == 100 )
                chat_list.positionViewAtEnd()
            else
            if( height == 0 )
                chat_list.positionViewAtEnd()
        }

        Behavior on height {
            NumberAnimation{ easing.type: Easing.OutBack; duration: 300 }
        }
    }

    SendFrame {
        id: send_frame
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        current: chat_view.current
    }

    Item {
        id: config_frame
        height: parent.height
        width: userConfig? parent.width : 0
        anchors.right: parent.right
        clip: true
        visible: width != 0

        onVisibleChanged: u_config.focus = visible

        Behavior on width {
            NumberAnimation{ easing.type: Easing.OutBack; duration: 400 }
        }

        Mirror {
            width: chat_view.width
            height: chat_view.height
            anchors.right: parent.right
            anchors.top: parent.top
            source: chat_list_frame
            visible: imageBack
        }

        Rectangle {
            anchors.fill: parent
            color: imageBack? "#88ffffff" : "#d9d9d9"
        }

        UserConfig {
            id: u_config
            width: chat_view.width
            height: chat_view.height
            anchors.left: parent.left
            anchors.top: parent.top
            onBackRequest: userConfig = false
            onChatRequest: main.current = uid
        }
    }

    Component {
        id: msg_act_component
        MsgAction {
            id: msg_action
            anchors.centerIn: parent
        }
    }

    Component {
        id: msg_component
        MsgItem {
            id: msg_item
            width: parent.width
            y: height
            scale: 1.1
            transformOrigin: Item.Center

            onContactSelected: {
                u_config.userId = uid
                chat_view.userConfig = true
            }

            Behavior on y {
                NumberAnimation{ easing.type: Easing.OutCubic; duration: chat_list.disableAnims? 0 : 600 }
            }
            Behavior on scale {
                NumberAnimation{ easing.type: Easing.OutCubic; duration: chat_list.disableAnims? 0 : 600 }
            }

            Component.onCompleted: {
                y = 0
                scale = 1
            }
        }
    }

    function showConfigure( uid ) {
        u_config.userId = uid
        if( u_config.userId == 0 )
            return

        userConfig = true
    }
}
