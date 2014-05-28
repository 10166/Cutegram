#define NOTIFY_ACT_SHOW 0
#define NOTIFY_ACT_MUTE 1
#define NOTIFY_ACT_RMND 2

#include "telegramgui.h"
#include "notification.h"
#include "telegram_macros.h"
#include "userdata.h"
#include "telegram/telegram.h"

#include <QtQml>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include <QHash>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPointer>
#include <QSettings>
#include <QQuickWindow>
#include <QDir>

QPointer<QSettings> tg_settings;

class TelegramGuiPrivate
{
public:
    Notification *notify;
    UserData *userdata;

    QSystemTrayIcon *sysTray;
    QQmlApplicationEngine *engine;
    Telegram *tg;

    char *args;

    QQuickWindow *root;
    QHash<int,bool> mutes;
};

TelegramGui::TelegramGui(QObject *parent) :
    QObject(parent)
{
    p = new TelegramGuiPrivate;
    p->engine = 0;
    p->tg = 0;
    p->args = QGuiApplication::arguments().first().toUtf8().data();

    QDir().mkpath(HOME_PATH);
    if( !tg_settings )
        tg_settings = new QSettings( HOME_PATH + "/telegram.conf", QSettings::IniFormat, this);

    p->notify = new Notification(this);

    connect( p->notify, SIGNAL(notifyAction(uint,QString)), SLOT(notify_action(uint,QString)) );

    qmlRegisterType<Enums>("org.sialan.telegram", 1, 0, "Enums");
}

QSettings *TelegramGui::settings()
{
    return tg_settings;
}

void TelegramGui::setMute(int id, bool stt)
{
    if( p->mutes.value(id) == stt )
        return;

    p->mutes.insert(id, stt);
    emit muted(id, stt);
}

bool TelegramGui::isMuted(int id) const
{
    return p->mutes.value(id);
}

int TelegramGui::desktopSession()
{
    static int result = -1;
    if( result != -1 )
        return result;

#ifdef Q_OS_MAC
    result = Enums::Mac;
#else
#ifdef Q_OS_WIN
    result = Enums::Windows;
#else
    static QString *desktop_session = 0;
    if( !desktop_session )
        desktop_session = new QString( qgetenv("DESKTOP_SESSION") );

    if( desktop_session->contains("kde",Qt::CaseInsensitive) )
        result = Enums::Kde;
    else
    if( QString(qgetenv("XDG_CURRENT_DESKTOP")).contains("unity",Qt::CaseInsensitive) )
        result = Enums::Unity;
    else
        result = Enums::Gnome;
#endif
#endif

    if( result == -1 )
        result = Enums::Unknown;

    return result;
}

void TelegramGui::start()
{
    if( p->engine )
        return;

    p->tg = new Telegram(1,&(p->args));
    p->userdata = new UserData(this);

    p->engine = new QQmlApplicationEngine(this);
    p->engine->rootContext()->setContextProperty( "Telegram", p->tg );
    p->engine->rootContext()->setContextProperty( "Gui", this );
    p->engine->rootContext()->setContextProperty( "UserData", p->userdata );
    p->engine->load(QUrl(QStringLiteral("qrc:///main.qml")));

    p->root = static_cast<QQuickWindow*>(p->engine->rootObjects().first());

    p->sysTray = new QSystemTrayIcon( QIcon(":/files/sys_tray.png"), this );
    p->sysTray->show();

    connect( p->sysTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(systray_action(QSystemTrayIcon::ActivationReason)) );
}

void TelegramGui::sendNotify(quint64 msg_id)
{
    QStringList actions;
    if( desktopSession() != Enums::Unity )
    {
        actions << QString("%1:%2").arg(NOTIFY_ACT_SHOW).arg(msg_id) << tr("Show");
        actions << QString("%1:%2").arg(NOTIFY_ACT_MUTE).arg(msg_id) << tr("Mute");
//        actions << QString("%1:%2").arg(NOTIFY_ACT_RMND).arg(msg_id) << tr("Mute & Remind");
    }

    int to_id = p->tg->messageToId(msg_id);
    int from_id = p->tg->messageFromId(msg_id);
    if( from_id == p->tg->me() )
        return;
    if( isMuted(to_id) || isMuted(from_id) )
        return;

    QString title = p->tg->messageFromName(msg_id);
    QString icon = p->tg->getPhotoPath(from_id);
    QString body = p->tg->messageBody(msg_id);
    if( p->tg->dialogIsChat(to_id) )
        title += tr("at %1").arg(p->tg->dialogTitle(to_id));
    else
    if( p->tg->dialogIsChat(from_id) )
        title += tr("at %1").arg(p->tg->dialogTitle(from_id));

    p->notify->sendNotify( title, body, icon, 0, 3000, actions );
}

void TelegramGui::notify_action(uint id, const QString &act)
{
    Q_UNUSED(id)
    const QStringList & splits = act.split(":");
    if( splits.count() != 2 )
        return;

    uint act_id = splits.at(0).toUInt();
    quint64 msg_id = splits.at(1).toULongLong();

    int to_id = p->tg->messageToId(msg_id);
    int from_id = p->tg->messageFromId(msg_id);
    int current = from_id;
    if( p->tg->dialogIsChat(to_id) )
        current = to_id;

    switch (act_id) {
    case NOTIFY_ACT_SHOW:
        p->root->setVisible( true );
        p->root->setProperty( "current", current );
        p->root->requestActivate();
        break;

    case NOTIFY_ACT_MUTE:
        setMute( current, true );
        break;

    case NOTIFY_ACT_RMND:
        break;
    }
}

void TelegramGui::systray_action(QSystemTrayIcon::ActivationReason act)
{
    switch( static_cast<int>(act) )
    {
    case QSystemTrayIcon::Trigger:
        if( p->root->isVisible() && p->root->isActive() )
            p->root->hide();
        else
        {
            p->root->setVisible( true );
            p->root->requestActivate();
        }
        break;

    case QSystemTrayIcon::Context:
        showContextMenu();
        break;
    }
}

void TelegramGui::showContextMenu()
{
    QMenu menu;
    menu.move( QCursor::pos() );

    QAction *show_act = menu.addAction( tr("Show") );
    menu.addSeparator();
    QAction *conf_act = menu.addAction( tr("Configure") );
    QAction *abut_act = menu.addAction( tr("About") );
    menu.addSeparator();
    QAction *exit_act = menu.addAction( tr("Exit") );
    QAction *res_act  = menu.exec();

    if( res_act == show_act )
    {
        p->root->setVisible( true );
        p->root->requestActivate();
    }
    else
    if( res_act == conf_act )
    {
        p->root->setVisible( true );
        p->root->requestActivate();
        p->root->setProperty( "configure", !p->root->property("configure").toBool() );
        p->root->setProperty( "focus", true );
    }
    else
    if( res_act == abut_act )
    {
        p->root->setVisible( true );
        p->root->requestActivate();
        p->root->setProperty( "about", !p->root->property("about").toBool() );
        p->root->setProperty( "focus", true );
    }
    else
    if( res_act == exit_act )
    {
        QCoreApplication::quit();
    }
}

TelegramGui::~TelegramGui()
{
    delete p;
}
