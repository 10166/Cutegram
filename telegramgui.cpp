#define NOTIFY_ACT_SHOW 0
#define NOTIFY_ACT_MUTE 1
#define NOTIFY_ACT_RMND 2

#include "telegramgui.h"
#include "notification.h"
#include "telegram/telegram.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include <QHash>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class TelegramGuiPrivate
{
public:
    Notification *notify;

    QSystemTrayIcon *sysTray;
    QQmlApplicationEngine *engine;
    Telegram *tg;

    char *args;

    QObject *root;
    QHash<int,bool> mutes;
};

TelegramGui::TelegramGui(QObject *parent) :
    QObject(parent)
{
    p = new TelegramGuiPrivate;
    p->engine = 0;
    p->tg = 0;
    p->args = QGuiApplication::arguments().first().toUtf8().data();

    p->notify = new Notification(this);

    connect( p->notify, SIGNAL(notifyAction(uint,QString)), SLOT(notify_action(uint,QString)) );
}

void TelegramGui::setMute(int id, bool stt)
{
    p->mutes.insert(id, stt);
}

bool TelegramGui::muted(int id) const
{
    return p->mutes.value(id);
}

void TelegramGui::start()
{
    if( p->engine )
        return;

    p->tg = new Telegram(1,&(p->args));

    p->engine = new QQmlApplicationEngine(this);
    p->engine->rootContext()->setContextProperty( "Telegram", p->tg );
    p->engine->rootContext()->setContextProperty( "Gui", this );
    p->engine->load(QUrl(QStringLiteral("qrc:///main.qml")));

    p->root = p->engine->rootObjects().first();

    p->sysTray = new QSystemTrayIcon( QIcon(":/files/sys_tray.png"), this );
//    p->sysTray->show();

    connect( p->sysTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(systray_action(QSystemTrayIcon::ActivationReason)) );
}

void TelegramGui::sendNotify(quint64 msg_id)
{
    QStringList actions;
    actions << QString("%1:%2").arg(NOTIFY_ACT_SHOW).arg(msg_id) << tr("Show");
    actions << QString("%1:%2").arg(NOTIFY_ACT_MUTE).arg(msg_id) << tr("Mute");
//    actions << QString("%1:%2").arg(NOTIFY_ACT_RMND).arg(msg_id) << tr("Mute & Remind");

    int to_id = p->tg->messageFromId(msg_id);
    int from_id = p->tg->messageFromId(msg_id);
    if( muted(to_id) || muted(from_id) )
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

    int to_id = p->tg->messageFromId(msg_id);
    int from_id = p->tg->messageFromId(msg_id);
    int current = from_id;
    if( p->tg->dialogIsChat(to_id) )
        current = to_id;

    switch (act_id) {
    case NOTIFY_ACT_SHOW:
        p->root->setProperty( "visible", true );
        p->root->setProperty( "current", current );
        break;

    case NOTIFY_ACT_MUTE:
        qDebug() << current << "Mutted";
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
        p->root->setProperty( "visible", !p->root->property("visible").toBool() );
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
    QAction *exit_act = menu.addAction( tr("Exit") );
    QAction *res_act  = menu.exec();

    if( res_act == show_act )
    {
        p->root->setProperty( "visible", !p->root->property("visible").toBool() );
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
