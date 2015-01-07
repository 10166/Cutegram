#ifndef DATABASECORE_H
#define DATABASECORE_H

#include <QObject>
#include <types/types.h>

class DbChat { public: DbChat(): chat(Chat::typeChatEmpty){} Chat chat; };
class DbUser { public: DbUser(): user(User::typeUserEmpty){} User user; };
class DbDialog { public: DbDialog(): dialog(){} Dialog dialog; };
class DbMessage { public: DbMessage(): message(){} Message message; };
class DbPeer { public: DbPeer(): peer(Peer::typePeerUser){} Peer peer; };

class DatabaseCorePrivate;
class DatabaseCore : public QObject
{
    Q_OBJECT
public:
    DatabaseCore(const QString &path, const QString &phoneNumber, QObject *parent = 0);
    ~DatabaseCore();

public slots:
    void reconnect();
    void disconnect();

    void insertUser(const DbUser &user);
    void insertChat(const DbChat &chat);
    void insertDialog(const DbDialog &dialog);
    void insertMessage(const DbMessage &message);

    void readFullDialogs();
    void readMessages(const DbPeer &peer, int offset, int limit);

signals:
    void userFounded(const DbUser &user);
    void chatFounded(const DbChat &chat);
    void dialogFounded(const DbDialog &dialog);
    void messageFounded(const DbMessage &message);

private:
    void readDialogs();
    void readUsers();
    void readChats();

    void init_buffer();
    void update_db();

    QList<qint32> stringToUsers(const QString &str);
    QString usersToString( const QList<qint32> &users );

    void insertAudio(const Audio &audio);
    void insertVideo(const Video &video);
    void insertDocument(const Document &document);
    void insertGeo(int id, const GeoPoint &geo);
    void insertPhoto(const Photo &photo);
    void insertPhotoSize(int pid, const QList<PhotoSize> &sizes);

    Audio readAudio(qint64 id);
    Video readVideo(qint64 id);
    Document readDocument(qint64 id);
    GeoPoint readGeo(qint64 id);
    Photo readPhoto(qint64 id);
    QList<PhotoSize> readPhotoSize(qint64 pid);

    void begin();
    void commit();

protected:
    void timerEvent(QTimerEvent *e);

private:
    DatabaseCorePrivate *p;
};

Q_DECLARE_METATYPE(DbUser)
Q_DECLARE_METATYPE(DbChat)
Q_DECLARE_METATYPE(DbDialog)
Q_DECLARE_METATYPE(DbMessage)
Q_DECLARE_METATYPE(DbPeer)

#endif // DATABASECORE_H
