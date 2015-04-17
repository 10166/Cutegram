#include "asemantools/asemanapplication.h"
#include "asemantools/asemanquickview.h"

#include "cutegram.h"
#include "compabilitytools.h"

#include <QMainWindow>
#include <QPalette>
#include <QNetworkProxy>

int main(int argc, char *argv[])
{
    qputenv("QT_LOGGING_RULES", "tg.*=false");

    AsemanApplication app(argc, argv);
    app.setApplicationName("Cutegram");
    app.setApplicationDisplayName("Cutegram");
    app.setApplicationVersion("v2.1.2");
    app.setOrganizationDomain("land.aseman");
    app.setOrganizationName("Aseman");
    app.setWindowIcon(QIcon(":/qml/Cutegram/files/icon.png"));
    app.setQuitOnLastWindowClosed(false);

    if(app.readSetting("Proxy/enable",false).toBool())
    {
        const int type = app.readSetting("Proxy/type",QNetworkProxy::HttpProxy).toInt();
        const QString host = app.readSetting("Proxy/host").toString();
        const quint16 port = app.readSetting("Proxy/port").toInt();
        const QString user = app.readSetting("Proxy/user").toString();
        const QString pass = app.readSetting("Proxy/pass").toString();

        QNetworkProxy proxy;
        proxy.setType( static_cast<QNetworkProxy::ProxyType>(type) );
        proxy.setHostName(host);
        proxy.setPort(port);
        proxy.setUser(user);
        proxy.setPassword(pass);
        QNetworkProxy::setApplicationProxy(proxy);
    }

#ifdef Q_OS_MAC
    QPalette palette;
    palette.setColor(QPalette::Highlight, "#0d80ec");
    palette.setColor(QPalette::HighlightedText, "#ffffff");
    app.setPalette(palette);
#endif

#ifdef DESKTOP_DEVICE
    if( !app.arguments().contains("--force") && app.isRunning() )
    {
        app.sendMessage("show");
        return 0;
    }
#endif

    CompabilityTools::version1();

    Cutegram cutegram;
    cutegram.start();

#ifdef DESKTOP_DEVICE
    QObject::connect( &app, SIGNAL(messageReceived(QString)), &cutegram, SLOT(incomingAppMessage(QString)) );
    QObject::connect( &app, SIGNAL(clickedOnDock())         , &cutegram, SLOT(incomingAppMessage())        );
#endif

    return app.exec();
}
