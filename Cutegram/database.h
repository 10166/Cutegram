#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>

class Peer;
class Message;
class User;
class Dialog;
class Chat;
class DbUser;
class DbDialog;
class DbMessage;
class DbChat;
class DatabasePrivate;
class Database : public QObject
{
    Q_OBJECT
public:
    Database(const QString & phoneNumber, QObject *parent = 0);
    ~Database();

public slots:
    void insertUser(const User &user);
    void insertChat(const Chat &chat);
    void insertDialog(const Dialog &dialog, bool encrypted);
    void insertMessage(const Message &message);
    void insertMediaEncryptedKeys(qint64 mediaId, const QByteArray &key, const QByteArray &iv);

    void readFullDialogs();
    void readMessages(const Peer &peer, int offset, int limit);

    void deleteMessage(qint64 msgId);
    void deleteDialog(qint64 dlgId);
    void deleteHistory(qint64 dlgId);

signals:
    void userFounded(const User &user);
    void chatFounded(const Chat &chat);
    void dialogFounded(const Dialog &dialog, bool encrypted);
    void messageFounded(const Message &message);
    void mediaKeyFounded(qint64 mediaId, const QByteArray &key, const QByteArray &iv);

private slots:
    void userFounded_slt(const DbUser &user);
    void chatFounded_slt(const DbChat &chat);
    void dialogFounded_slt(const DbDialog &dialog, bool encrypted);
    void messageFounded_slt(const DbMessage &message);

private:
    DatabasePrivate *p;
};

#endif // DATABASE_H
