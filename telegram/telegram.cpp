#include "telegram.h"
#include "telegramthread.h"
#include "strcuts.h"

#include <QMap>
#include <QTimerEvent>
#include <QFontMetricsF>
#include <QCoreApplication>
#include <QFileDialog>
#include <QDebug>

class TelegramPrivate
{
public:
    TelegramThread *tg_thread;

    int update_dialog_timer_id;
    bool update_dialog_again;

    int update_contact_timer_id;
    bool update_contact_again;

    Enums::waitAndGet last_wait_and_get;

    bool started;
    bool authenticating;

    QSet<int> loaded_users_info;
    QSet<int> loaded_chats_info;
};

Telegram *sortDialogList_tmp_obj = 0;
bool sortDialogList(int m1, int m2)
{
    return sortDialogList_tmp_obj->dialogMsgDate(m1) > sortDialogList_tmp_obj->dialogMsgDate(m2);
}

Telegram::Telegram(int argc, char **argv, QObject *parent) :
    QObject(parent)
{
    p = new TelegramPrivate;
    p->update_dialog_again = false;
    p->update_dialog_timer_id = 0;
    p->update_contact_again = false;
    p->update_contact_timer_id = 0;
    p->authenticating = false;
    p->started = false;
    p->last_wait_and_get = Enums::NoWaitAndGet;

    p->tg_thread = new TelegramThread(argc,argv);

    connect( p->tg_thread, SIGNAL(contactsChanged())                   , SIGNAL(meChanged())                          );
    connect( p->tg_thread, SIGNAL(contactsChanged())                   , SIGNAL(contactsChanged())                    );
    connect( p->tg_thread, SIGNAL(dialogsChanged())                    , SIGNAL(dialogsChanged())                     );
    connect( p->tg_thread, SIGNAL(incomingMsg(qint64))                 , SIGNAL(incomingMsg(qint64))                  );
    connect( p->tg_thread, SIGNAL(incomingNewMsg(qint64))              , SIGNAL(incomingNewMsg(qint64))               );
    connect( p->tg_thread, SIGNAL(userIsTyping(int,int))               , SIGNAL(userIsTyping(int,int))                );
    connect( p->tg_thread, SIGNAL(userStatusChanged(int,int,QDateTime)), SIGNAL(userStatusChanged(int,int,QDateTime)) );
    connect( p->tg_thread, SIGNAL(msgChanged(qint64))                  , SIGNAL(msgChanged(qint64))                   );
    connect( p->tg_thread, SIGNAL(msgSent(qint64,qint64))              , SIGNAL(msgSent(qint64,qint64))               );
    connect( p->tg_thread, SIGNAL(userPhotoChanged(int))               , SIGNAL(userPhotoChanged(int))                );
    connect( p->tg_thread, SIGNAL(chatPhotoChanged(int))               , SIGNAL(chatPhotoChanged(int))                );
    connect( p->tg_thread, SIGNAL(fileUploaded(int,QString))           , SIGNAL(fileUploaded(int,QString))            );
    connect( p->tg_thread, SIGNAL(fileUploading(int,QString,qreal))    , SIGNAL(fileUploading(int,QString,qreal))     );
    connect( p->tg_thread, SIGNAL(fileUserUploaded(int))               , SIGNAL(fileUserUploaded(int))                );
    connect( p->tg_thread, SIGNAL(fileUserUploading(int,qreal))        , SIGNAL(fileUserUploading(int,qreal))         );
    connect( p->tg_thread, SIGNAL(msgFileDownloaded(qint64))           , SIGNAL(msgFileDownloaded(qint64))            );
    connect( p->tg_thread, SIGNAL(msgFileDownloading(qint64,qreal))    , SIGNAL(msgFileDownloading(qint64,qreal))     );
    connect( p->tg_thread, SIGNAL(messageDeleted(qint64))              , SIGNAL(messageDeleted(qint64))               );
    connect( p->tg_thread, SIGNAL(messageRestored(qint64))             , SIGNAL(messageRestored(qint64))              );
    connect( p->tg_thread, SIGNAL(registeringStarted())                , SLOT(registeringStarted())                   );
    connect( p->tg_thread, SIGNAL(registeringFinished())               , SLOT(registeringFinished())                  );
    connect( p->tg_thread, SIGNAL(registeringInvalidCode())            , SIGNAL(registeringInvalidCode())             );
    connect( p->tg_thread, SIGNAL(waitAndGet(int))                     , SLOT(_waitAndGet(int))                       );
    connect( p->tg_thread, SIGNAL(tgStarted())                         , SLOT(_startedChanged())                      );

    p->tg_thread->start();
}

QList<int> Telegram::contactListUsers() const
{
    return p->tg_thread->contacts().keys();
}

UserClass Telegram::contact(int id) const
{
    return p->tg_thread->contacts().value(id);
}

QString Telegram::contactFirstName(int id) const
{
    return contact(id).firstname;
}

QString Telegram::contactLastName(int id) const
{
    return contact(id).lastname;
}

QString Telegram::contactPhone(int id) const
{
    return contact(id).phone;
}

int Telegram::contactUid(int id) const
{
    return contact(id).user_id;
}

int Telegram::contactState(int id) const
{
    return contact(id).state;
}

QDateTime Telegram::contactLastTime(int id) const
{
    return contact(id).lastTime;
}

QString Telegram::contactTitle(int id) const
{
    return contactFirstName(id) + " " + contactLastName(id);
}

QString Telegram::contactLastSeenText(int id) const
{
    switch( contactState(id) )
    {
    case Enums::Online:
        return tr("Online");
        break;

    case Enums::Offline:
    case Enums::NotOnlineYet:
        return tr("Last seen") + " " + convertDateToString( contactLastTime(id) );
        break;
    }

    return QString();
}

QList<int> Telegram::dialogListIds()
{
    QList<int> res = p->tg_thread->dialogs().keys();

    sortDialogList_tmp_obj = this;
    qSort( res.begin(), res.end(), sortDialogList );

    return res;
}

DialogClass Telegram::dialog(int id) const
{
    return p->tg_thread->dialogs().value(id);
}

bool Telegram::dialogIsChat(int id) const
{
    return dialog(id).is_chat;
}

QString Telegram::dialogChatTitle(int id) const
{
    return dialog(id).chatClass.title;
}

int Telegram::dialogChatAdmin(int id) const
{
    return dialog(id).chatClass.admin;
}

qint64 Telegram::dialogChatPhotoId(int id) const
{
    return dialog(id).chatClass.photoId;
}

int Telegram::dialogChatUsersNumber(int id) const
{
    return dialog(id).chatClass.users_num;
}

QDateTime Telegram::dialogChatDate(int id) const
{
    return dialog(id).chatClass.date;
}

QString Telegram::dialogUserName(int id) const
{
    return dialog(id).userClass.username;
}

QString Telegram::dialogUserFirstName(int id) const
{
    return dialog(id).userClass.firstname;
}

QString Telegram::dialogUserLastName(int id) const
{
    return dialog(id).userClass.lastname;
}

QString Telegram::dialogUserPhone(int id) const
{
    return dialog(id).userClass.phone;
}

int Telegram::dialogUserUid(int id) const
{
    return dialog(id).userClass.user_id;
}

int Telegram::dialogUserState(int id) const
{
    return dialog(id).userClass.state;
}

QDateTime Telegram::dialogUserLastTime(int id) const
{
    return dialog(id).userClass.lastTime;
}

QString Telegram::dialogUserTitle(int id) const
{
    return dialogUserFirstName(id) + " " + dialogUserLastName(id);
}

QString Telegram::dialogTitle(int id) const
{
    return dialogIsChat(id)? dialogChatTitle(id) : dialogUserFirstName(id) + " " + dialogUserLastName(id);
}

int Telegram::dialogUnreadCount(int id) const
{
    return dialog(id).unread;
}

QDateTime Telegram::dialogMsgDate(int id) const
{
    return dialog(id).msgDate;
}

QString Telegram::dialogMsgLast(int id) const
{
    return dialog(id).msgLast;
}

bool Telegram::isDialog(int id) const
{
    return p->tg_thread->dialogs().contains(id);
}

QString Telegram::title(int id) const
{
    return isDialog(id)? dialogTitle(id) : contactTitle(id);
}

QString Telegram::getPhotoPath(int id) const
{
    return p->tg_thread->photos().value(id);
}

QList<qint64> Telegram::messageIds() const
{
    return p->tg_thread->messages().keys();
}

QStringList Telegram::messagesOf(int current) const
{
    QMap<qint64,QString> res;
    bool is_chat = dialogIsChat(current);

    const QHash<qint64,MessageClass> & msgs = p->tg_thread->messages();
    QHashIterator<qint64,MessageClass> i(msgs);
    while( i.hasNext() )
    {
        i.next();
        const MessageClass & msg = i.value();
        if( is_chat && msg.to_id != current && msg.to_id != 0 )
            continue;
        else
        if( !is_chat )
        {
            if( msg.out && msg.to_id != current && msg.to_id != 0 )
                continue;
            else
            if( !msg.out && msg.from_id != current && msg.from_id != 0 )
                continue;
            else
            if( dialogIsChat(msg.to_id) || dialogIsChat(msg.from_id) )
                continue;
        }

        res.insert( msg.date.toMSecsSinceEpoch(), QString::number(msg.msg_id) );
    }

    return res.values();
}

QStringList Telegram::messageIdsStringList() const
{
    const QList<qint64> & msgs = messageIds();
    QStringList result;
    foreach( qint64 id, msgs )
        result << QString::number(id);

    return result;
}

MessageClass Telegram::message(qint64 id) const
{
    return p->tg_thread->messages().value(id);
}

int Telegram::messageForwardId(qint64 id) const
{
    return message(id).fwd_id;
}

QDateTime Telegram::messageForwardDate(qint64 id) const
{
    return message(id).fwd_date;
}

bool Telegram::messageOut(qint64 id) const
{
    return message(id).out;
}

int Telegram::messageUnread(qint64 id) const
{
    return message(id).unread;
}

QDateTime Telegram::messageDate(qint64 id) const
{
    return message(id).date;
}

int Telegram::messageService(qint64 id) const
{
    return message(id).service;
}

QString Telegram::messageBody(qint64 id) const
{
    return message(id).message;
}

qreal Telegram::messageBodyTextWidth(qint64 id) const
{
    const QString & txt = messageBody(id);
    QFontMetricsF metric = QFontMetricsF( QFont() );
    return metric.width(txt);
}

int Telegram::messageFromId(qint64 id) const
{
    return message(id).from_id;
}

int Telegram::messageToId(qint64 id) const
{
    return message(id).to_id;
}

QString Telegram::messageFromName(qint64 id) const
{
    const MessageClass & msg = message(id);
    return msg.firstName + " " + msg.lastName;
}

qint64 Telegram::messageMediaType(qint64 id) const
{
    return message(id).mediaType;
}

bool Telegram::messageIsPhoto(qint64 id) const
{
    return messageMediaType(id) == Enums::MediaPhoto;
}

QString Telegram::messageMediaFile(qint64 id) const
{
    return message(id).mediaFile;
}

bool Telegram::messageIsDeleted(qint64 id) const
{
    return message(id).deleted;
}

int Telegram::me() const
{
    return p->tg_thread->me();
}

bool Telegram::started() const
{
    return p->started;
}

QString Telegram::convertDateToString(const QDateTime &date) const
{
    const QDateTime & today = QDateTime::currentDateTime();
    if( date.date().year() != today.date().year() )
        return date.date().toString("yyyy MMM d");
    else
    if( date.date().month() != today.date().month() )
        return date.date().toString("MMM d");
    else
    if( date.date().day() != today.date().day() )
        return date.date().toString("MMM d");
    else
        return date.time().toString("hh:mm");
}

int Telegram::lastWaitAndGet() const
{
    return p->last_wait_and_get;
}

bool Telegram::authenticating() const
{
    return p->authenticating;
}

void Telegram::updateContactList()
{
    p->tg_thread->contactList();
}

void Telegram::updateDialogList()
{
    p->tg_thread->dialogList();
}

void Telegram::updateDialogListUsingTimer()
{
    if( p->update_dialog_timer_id )
    {
        p->update_dialog_again = true;
        return;
    }

    p->update_dialog_again = false;
    p->update_dialog_timer_id = startTimer(1000);
}

void Telegram::updateContactListUsingTimer()
{
    if( p->update_contact_timer_id )
    {
        p->update_contact_timer_id = true;
        return;
    }

    p->update_contact_again = false;
    p->update_contact_timer_id = startTimer(1000);
}

void Telegram::getHistory(int id, int count)
{
    p->tg_thread->getHistory(id,count);
}

void Telegram::sendMessage(int id, const QString &msg)
{
    p->tg_thread->sendMessage(id,msg);
}

void Telegram::forwardMessage(qint64 msg_id, int user_id)
{
    p->tg_thread->forwardMessage(msg_id,user_id);
}

void Telegram::deleteMessage(qint64 msg_id)
{
    p->tg_thread->deleteMessage(msg_id);
}

void Telegram::restoreMessage(qint64 msg_id)
{
    p->tg_thread->restoreMessage(msg_id);
}

void Telegram::loadUserInfo(int userId)
{
    if( p->loaded_users_info.contains(userId) )
        return;

    p->tg_thread->loadUserInfo(userId);
    p->loaded_users_info.insert(userId);
}

void Telegram::loadChatInfo(int chatId)
{
    if( p->loaded_chats_info.contains(chatId) )
        return;

    p->tg_thread->loadChatInfo(chatId);
    p->loaded_chats_info.insert(chatId);
}

void Telegram::loadPhoto(qint64 msg_id)
{
    p->tg_thread->loadPhoto(msg_id);
}

void Telegram::sendFile(int dId, const QString &file)
{
    p->tg_thread->sendFile(dId,file);
}

bool Telegram::sendFileDialog(int dId)
{
    const QString & file = QFileDialog::getOpenFileName();
    if( file.isEmpty() )
        return false;

    sendFile(dId, file);
    return true;
}

void Telegram::markRead(int dId)
{
    p->tg_thread->markRead(dId);
}

void Telegram::setStatusOnline(bool stt)
{
    p->tg_thread->setStatusOnline(stt);
}

void Telegram::createChat(const QString &title, int user_id)
{
    p->tg_thread->createChat(title, user_id);
}

void Telegram::createSecretChat(int user_id)
{
    p->tg_thread->createSecretChat(user_id);
}

void Telegram::renameChat(int chat_id, const QString &new_title)
{
    p->tg_thread->renameChat(chat_id, new_title);
}

void Telegram::chatAddUser(int chat_id, int user_id)
{
    p->tg_thread->chatAddUser(chat_id, user_id);
}

void Telegram::chatDelUser(int chat_id, int user_id)
{
    p->tg_thread->chatDelUser(chat_id, user_id);
}

void Telegram::search(int user_id, const QString &keyword)
{
    p->tg_thread->search( user_id, keyword );
}

void Telegram::globalSearch(const QString &keyword)
{
    p->tg_thread->globalSearch(keyword);
}

void Telegram::waitAndGetCallback(Enums::waitAndGet type, const QVariant &var)
{
    p->tg_thread->waitAndGetCallback(type, var);
    _waitAndGet(Enums::CheckingState);
}

void Telegram::waitAndGetPhoneCallBack(const QString &phone)
{
    WaitGetPhone v;
    v.phone = phone;

    waitAndGetCallback( Enums::PhoneNumber, QVariant::fromValue<WaitGetPhone>(v) );
}

void Telegram::waitAndGetAuthCodeCallBack(const QString &code, bool call_request)
{
    WaitGetAuthCode v;
    v.code = code;
    v.request_phone = call_request;

    waitAndGetCallback( Enums::AuthCode, QVariant::fromValue<WaitGetAuthCode>(v) );
}

void Telegram::waitAndGetUserInfoCallBack(const QString &fname, const QString &lname)
{
    WaitGetUserDetails v;
    v.firstname = fname;
    v.lastname = lname;

    waitAndGetCallback( Enums::UserDetails, QVariant::fromValue<WaitGetUserDetails>(v) );
}

void Telegram::_waitAndGet(int type)
{
    Enums::waitAndGet wg = static_cast<Enums::waitAndGet>(type);
    p->last_wait_and_get = wg;
    emit waitAndGetChanged();
}

void Telegram::_startedChanged()
{
    p->started = true;
    emit startedChanged();
}

void Telegram::registeringStarted()
{
    p->authenticating = true;
    emit authenticatingChanged();
}

void Telegram::registeringFinished()
{
    p->authenticating = false;
    emit authenticatingChanged();
}

void Telegram::timerEvent(QTimerEvent *e)
{
    if( e->timerId() == p->update_dialog_timer_id )
    {
        updateDialogList();
        if( p->update_dialog_again )
        {
            p->update_dialog_again = false;
            return;
        }

        p->update_dialog_again = false;
        killTimer(p->update_dialog_timer_id);
        p->update_dialog_timer_id = 0;
    }
    else
    if( e->timerId() == p->update_contact_timer_id )
    {
        updateContactList();
        if( p->update_contact_again )
        {
            p->update_contact_again = false;
            return;
        }

        p->update_contact_again = false;
        killTimer(p->update_contact_timer_id);
        p->update_contact_timer_id = 0;
    }
    else
        QObject::timerEvent(e);
}

Telegram::~Telegram()
{
    delete p;
}
