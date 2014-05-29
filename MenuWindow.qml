import QtQuick 2.0
import QtQuick.Window 2.1
import QtGraphicalEffects 1.0

Window {
    id: menu_win
    width: 300
    height: 400
    color: "#00000000"
    visible: menu_frame.visible

    property bool started: false
    property variant item

    property real shadowSize: 42

    property int duration: 350

    onItemChanged: {
        if( privates.oldItem )
            privates.oldItem.destroy()
        if( item ) {
            item.parent = menu_main_item
            item.anchors.fill = menu_main_item
        }

        privates.oldItem = item
    }

    QtObject {
        id: privates
        property variant oldItem
    }

    MouseArea {
        anchors.fill: parent
        onClicked: menu_win.stop()
    }

    Item {
        id: menu_frame
        x: 200
        y: 200
        width: 200
        height: 300
        transformOrigin: Item.Top
        scale: menu_win.started? 1 : 0.1
        opacity: menu_win.started? 1 : 0
        visible: opacity != 0

        DropShadow {
            anchors.fill: menu_item
            radius: shadowSize
            samples: 32
            cached: true
            spread: 0.5
            color: "#80000000"
            source: menu_item
        }

        Image {
            width: 32
            height: 18
            anchors.horizontalCenter: menu_item.horizontalCenter
            anchors.bottom: menu_item.top
            anchors.bottomMargin: -shadowSize
            sourceSize: Qt.size(width,height)
            source: "files/pointer.png"
        }

        Item {
            id: menu_item
            anchors.fill: parent
            anchors.margins: -shadowSize

            Rectangle {
                id: menu_main_item
                anchors.fill: parent
                anchors.margins: shadowSize
                color: "#ffffff"
                radius: 3
            }
        }

        Behavior on scale {
            NumberAnimation{ easing.type: Easing.OutBack; duration: menu_win.duration }
        }
        Behavior on opacity {
            NumberAnimation{ easing.type: Easing.OutCubic; duration: menu_win.duration }
        }
    }

    Timer {
        id: obj_destroyer
        interval: menu_win.duration
        repeat: false
        onTriggered: if( menu_win.item ) menu_win.item.destroy()
    }

    function start( component, x, y, w, h ) {
        var sz = Gui.screenSize()
        menu_win.width = sz.width
        menu_win.height = sz.height

        menu_frame.width = w
        menu_frame.height = y+h+shadowSize>height? height-y-shadowSize : h
        menu_frame.x = x - 1*menu_frame.width/2
        menu_frame.y = y
        item = component.createObject(menu_main_item)
        started = true

        return item
    }

    function stop() {
        started = false
        obj_destroyer.restart()
    }
}
