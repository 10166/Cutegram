/*
    Copyright (C) 2014 Aseman
    http://aseman.co

    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define LOAD_STEP_COUNT 50

#include "telegrammessagesmodel.h"
#include "telegramqml.h"
#include "database.h"
#include "objects/types.h"

#include <telegram.h>
#include <QPointer>

class TelegramMessagesModelPrivate
{
public:
    TelegramQml *telegram;
    bool initializing;
    bool refreshing;

    QList<qint64> messages;
    QPointer<DialogObject> dialog;

    int load_count;
    int load_limit;
};

TelegramMessagesModel::TelegramMessagesModel(QObject *parent) :
    QAbstractListModel(parent)
{
    p = new TelegramMessagesModelPrivate;
    p->telegram = 0;
    p->initializing = false;
    p->refreshing = false;
    p->load_count = 0;
    p->load_limit = 0;
}

QObject *TelegramMessagesModel::telegram() const
{
    return p->telegram;
}

void TelegramMessagesModel::setTelegram(QObject *tgo)
{
    TelegramQml *tg = static_cast<TelegramQml*>(tgo);
    if( p->telegram == tg )
        return;

    p->telegram = tg;
    p->initializing = tg;
    emit telegramChanged();
    emit initializingChanged();
    if( !p->telegram )
        return;

    connect( p->telegram, SIGNAL(messagesChanged()), SLOT(messagesChanged()) );

    init();
}

DialogObject *TelegramMessagesModel::dialog() const
{
    return p->dialog;
}

void TelegramMessagesModel::setDialog(DialogObject *dlg)
{
    if( p->dialog == dlg )
        return;

    p->dialog = dlg;
    emit dialogChanged();

    beginResetModel();
    p->messages.clear();
    endResetModel();

    if( !p->dialog )
        return;
    if( !p->dialog->peer()->chatId() && !p->dialog->peer()->userId() )
        return;

    init();
}

void TelegramMessagesModel::init()
{
    if( !p->dialog )
        return;
    if( !p->telegram )
        return;
    if( p->dialog == p->telegram->nullDialog() )
        return;

    p->load_count = 0;
    p->load_limit = LOAD_STEP_COUNT;
    loadMore(true);
    messagesChanged();

    p->refreshing = true;
    emit refreshingChanged();
}

void TelegramMessagesModel::refresh()
{
    if( !p->telegram )
        return;
    if( !p->dialog )
        return;

    const InputPeer & peer = inputPeer();

    Telegram *tgObject = p->telegram->telegram();
    tgObject->messagesGetHistory(peer, 0, 0, LOAD_STEP_COUNT );

    p->telegram->database()->readMessages(TelegramMessagesModel::peer(), 0, LOAD_STEP_COUNT);
}

void TelegramMessagesModel::loadMore(bool force)
{
    if( !p->telegram )
        return;
    if( !p->dialog )
        return;
    if( !force && p->refreshing )
        return;
    if( !force && p->messages.count() == 0 )
        return;

    const InputPeer & peer = inputPeer();

    p->load_limit = p->load_count + LOAD_STEP_COUNT;

    Telegram *tgObject = p->telegram->telegram();
    tgObject->messagesGetHistory(peer, p->load_count, 0, p->load_limit );

    p->telegram->database()->readMessages(TelegramMessagesModel::peer(), p->load_count, LOAD_STEP_COUNT);

    p->refreshing = true;
    emit refreshingChanged();
}

void TelegramMessagesModel::sendMessage(const QString &msg)
{
    if( !p->telegram )
        return;
    if( !p->dialog )
        return;

    qint32 did = p->dialog->peer()->classType()==Peer::typePeerChat? p->dialog->peer()->chatId() : p->dialog->peer()->userId();
    p->telegram->sendMessage(did, msg);
}

void TelegramMessagesModel::setReaded()
{
    if( p->telegram->invisible() )
        return;

    p->telegram->telegram()->messagesReadHistory(inputPeer());
    p->dialog->setUnreadCount(0);
}

qint64 TelegramMessagesModel::id(const QModelIndex &index) const
{
    int row = index.row();
    return p->messages.at(row);
}

int TelegramMessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return p->messages.count();
}

QVariant TelegramMessagesModel::data(const QModelIndex &index, int role) const
{
    QVariant res;
    const qint64 key = id(index);
    switch( role )
    {
    case ItemRole:
        res = QVariant::fromValue<MessageObject*>(p->telegram->message(key));
        break;
    }

    return res;
}

QHash<qint32, QByteArray> TelegramMessagesModel::roleNames() const
{
    QHash<qint32, QByteArray> *res = 0;
    if( res )
        return *res;

    res = new QHash<qint32, QByteArray>();
    res->insert( ItemRole, "item");
    return *res;
}

int TelegramMessagesModel::count() const
{
    return p->messages.count();
}

bool TelegramMessagesModel::initializing() const
{
    return p->initializing;
}

bool TelegramMessagesModel::refreshing() const
{
    return p->refreshing;
}

InputPeer TelegramMessagesModel::inputPeer() const
{
    bool isChat = p->dialog->peer()->classType()==Peer::typePeerChat;
    InputPeer peer(isChat? InputPeer::typeInputPeerChat : InputPeer::typeInputPeerContact);
    peer.setChatId(p->dialog->peer()->chatId());
    peer.setUserId(p->dialog->peer()->userId());
    return peer;
}

Peer TelegramMessagesModel::peer() const
{
    Peer peer( static_cast<Peer::PeerType>(p->dialog->peer()->classType()) );
    peer.setChatId(p->dialog->peer()->chatId());
    peer.setUserId(p->dialog->peer()->userId());
    return peer;
}

void TelegramMessagesModel::messagesChanged()
{
    p->refreshing = false;
    emit refreshingChanged();

    if( !p->dialog )
        return;

    qint32 did = p->dialog->peer()->classType()==Peer::typePeerChat? p->dialog->peer()->chatId() : p->dialog->peer()->userId();
    const QList<qint64> & messages = p->telegram->messages(did).mid(0,p->load_limit);

    for( int i=0 ; i<p->messages.count() ; i++ )
    {
        const qint64 dId = p->messages.at(i);
        if( messages.contains(dId) )
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        p->messages.removeAt(i);
        i--;
        endRemoveRows();
    }


    QList<qint64> temp_msgs = messages;
    for( int i=0 ; i<temp_msgs.count() ; i++ )
    {
        const qint64 dId = temp_msgs.at(i);
        if( p->messages.contains(dId) )
            continue;

        temp_msgs.removeAt(i);
        i--;
    }
    while( p->messages != temp_msgs )
        for( int i=0 ; i<p->messages.count() ; i++ )
        {
            const qint64 dId = p->messages.at(i);
            int nw = temp_msgs.indexOf(dId);
            if( i == nw )
                continue;

            beginMoveRows( QModelIndex(), i, i, QModelIndex(), nw>i?nw+1:nw );
            p->messages.move( i, nw );
            endMoveRows();
        }


    for( int i=0 ; i<messages.count() ; i++ )
    {
        const qint64 dId = messages.at(i);
        if( p->messages.contains(dId) )
            continue;

        beginInsertRows(QModelIndex(), i, i );
        p->messages.insert( i, dId );
        endInsertRows();
    }

    p->load_count = p->messages.count();
    emit countChanged();
}

TelegramMessagesModel::~TelegramMessagesModel()
{
    delete p;
}
