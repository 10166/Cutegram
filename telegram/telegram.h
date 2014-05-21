#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <QObject>
#include <QStringList>
#include <QDateTime>

#include "telegram/strcuts.h"

class TelegramPrivate;
class Telegram : public QObject
{
    Q_OBJECT
public:
    Telegram(int argc, char **argv, QObject *parent = 0);
    ~Telegram();

    Q_INVOKABLE QList<int> contactListUsers() const;
    UserClass contact(int id ) const;
    Q_INVOKABLE QString contactFirstName(int id) const;
    Q_INVOKABLE QString contactLastName(int id) const;
    Q_INVOKABLE QString contactPhone(int id) const;
    Q_INVOKABLE int contactUid(int id) const;
    Q_INVOKABLE qint64 contactPhotoId(int id) const;
    Q_INVOKABLE int contactState(int id) const;
    Q_INVOKABLE QDateTime contactLastTime(int id) const;
    Q_INVOKABLE QString contactTitle(int id);

    Q_INVOKABLE QList<int> dialogListIds();
    DialogClass dialog( int id ) const;
    Q_INVOKABLE bool dialogIsChat( int id ) const;
    Q_INVOKABLE QString dialogChatTitle( int id ) const;
    Q_INVOKABLE int dialogChatAdmin( int id ) const;
    Q_INVOKABLE qint64 dialogChatPhotoId( int id ) const;
    Q_INVOKABLE int dialogChatUsersNumber( int id ) const;
    Q_INVOKABLE QDateTime dialogChatDate( int id ) const;
    Q_INVOKABLE QString dialogUserName( int id ) const;
    Q_INVOKABLE QString dialogUserFirstName(int id) const;
    Q_INVOKABLE QString dialogUserLastName(int id) const;
    Q_INVOKABLE QString dialogUserPhone(int id) const;
    Q_INVOKABLE int dialogUserUid(int id) const;
    Q_INVOKABLE qint64 dialogUserPhotoId(int id) const;
    Q_INVOKABLE int dialogUserState(int id) const;
    Q_INVOKABLE QDateTime dialogUserLastTime(int id) const;
    Q_INVOKABLE QString dialogUserTitle(int id) const;
    Q_INVOKABLE QString dialogTitle( int id ) const;
    Q_INVOKABLE int dialogUnreadCount( int id ) const;
    Q_INVOKABLE QDateTime dialogMsgDate( int id ) const;
    Q_INVOKABLE QString dialogMsgLast( int id ) const;

    Q_INVOKABLE QList<qint64> messageIds() const;
    Q_INVOKABLE QStringList messageIdsStringList() const;
    MessageClass message( qint64 id ) const;
    Q_INVOKABLE int messageForwardId( qint64 id ) const;
    Q_INVOKABLE QDateTime messageForwardDate( qint64 id ) const;
    Q_INVOKABLE int messageOut( qint64 id ) const;
    Q_INVOKABLE int messageUnread( qint64 id ) const;
    Q_INVOKABLE QDateTime messageDate( qint64 id ) const;
    Q_INVOKABLE int messageService( qint64 id ) const;
    Q_INVOKABLE QString messageBody( qint64 id ) const;
    Q_INVOKABLE qreal messageBodyTextWidth( qint64 id ) const;
    Q_INVOKABLE int messageFromId( qint64 id ) const;
    Q_INVOKABLE int messageToId( qint64 id ) const;
    Q_INVOKABLE QString messageFromName( qint64 id ) const;

    Q_INVOKABLE QString convertDateToString( const QDateTime & date );

public slots:
    void updateContactList();
    void updateDialogList();
    void updateDialogListUsingTimer();

    void getHistory( int id, int count );
    void sendMessage( int id, const QString & msg );

    void loadUserInfo( int userId );
    void loadUserPhoto( int userId );

    void setStatusOnline( bool stt );

signals:
    void contactsChanged();
    void dialogsChanged();
    void incomingMsg( qint64 msg_id );
    void userIsTyping( int chat_id, int user_id );
    void userStatusChanged( int user_id, int status, const QDateTime & when );
    void msgChanged( qint64 msg_id );
    void msgSent( qint64 old_id, qint64 msg_id );
    void started();

protected:
    void timerEvent(QTimerEvent *e);

private:
    TelegramPrivate *p;
};

#endif // TELEGRAM_H
