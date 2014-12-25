import QtQuick 2.2
import AsemanTools 1.0
import Cutegram 1.0

AsemanMain {
    id: main
    width: AsemanApp.readSetting("General/width", 1024)
    height: AsemanApp.readSetting("General/height", 600)
    color: "#333333"
    mainFrame: main_frame
    focus: true
    masterPalette.colorGroup: SystemPalette.Active

    property variant authDialog
    property alias profiles: profile_model
    property alias emojis: emojis_obj

    property bool aboutMode: false

    property color backColor0: "#eeeeee"
    property color backColor1: "#cccccc"
    property color backColor2: "#fafafa"
    property color textColor0: "#111111"
    property color textColor1: "#333333"
    property color textColor2: "#888888"

    onWidthChanged: size_save_timer.restart()
    onHeightChanged: size_save_timer.restart()

    onAboutModeChanged: {
        if(aboutMode)
            BackHandler.pushHandler(about, about.back)
        else
            BackHandler.removeHandler(about)
    }

    Keys.onEscapePressed: {
        AsemanApp.back()
    }

    Connections {
        target: Cutegram
        onBackRequest: AsemanApp.back()
        onAboutAsemanRequest: qlist.currentIndex = 0
    }

    Connections {
        target: AsemanApp
        onBackRequest: {
            var res = BackHandler.back()
            if( !res && !Devices.isDesktop )
                Cutegram.close()
        }
    }

    Connections {
        target: View
        onActiveChanged: {
            AsemanApp.setSetting("General/lastWindowState", View.active)
        }
    }

    Timer {
        id: size_save_timer
        interval: 1000
        onTriggered: {
            AsemanApp.setSetting("General/width", width)
            AsemanApp.setSetting("General/height", height)
        }
    }

    Emojis {
        id: emojis_obj
        currentTheme: "twitter"
    }

    ProfilesModel {
        id: profile_model
    }

    Timer {
        id: init_timer
        interval: 1500
        Component.onCompleted: start()
        onTriggered: {
            if( profiles.count == 0 )
                qlist.currentIndex = 3
            else
                qlist.currentIndex = 2
        }
    }

    AboutCutegram {
        id: about
        anchors.fill: parent

        function back() {
            aboutMode = false
        }
    }

    Item {
        id: main_frame
        width: parent.width
        height: parent.height
        y: aboutMode? height : 0

        Behavior on y {
            NumberAnimation{ easing.type: Easing.OutCubic; duration: 400 }
        }

        QueueList {
            id: qlist
            anchors.fill: parent
            components: [aseman_about_component, splash_component, accounts_frame, auth_dlg_component]
            currentIndex: 1
            onCurrentIndexChanged: {
                prevIndex = tmpIndex
                tmpIndex = currentIndex
            }

            property int tmpIndex: 0
            property int prevIndex: 0
        }
    }

    Component {
        id: auth_dlg_component
        AuthenticateDialog {
            anchors.fill: parent
            onAccepted: {
                var item = profiles.add(number)
                item.name = "AA"
                qlist.currentIndex = 2
            }
        }
    }

    Component {
        id: accounts_frame
        AccountsTabFrame {
            anchors.fill: parent
            property bool onceInstance: true
        }
    }

    Component {
        id: splash_component
        CutegramSplash {
            id: splash
            anchors.fill: parent
        }
    }

    Component {
        id: aseman_about_component
        AsemanAbout {
            id: aseman_about
            anchors.fill: parent
            Component.onCompleted: BackHandler.pushHandler(aseman_about, aseman_about.back)

            function back() {
                qlist.currentIndex = qlist.prevIndex
            }
        }
    }

    function addAccount() {
        qlist.currentIndex = 3
        BackHandler.pushHandler(main, main.backToAccounts )
    }

    function backToAccounts() {
        qlist.currentIndex = 2
        BackHandler.removeHandler(main)
    }
}
