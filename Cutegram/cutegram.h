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

#ifndef CUTEGRAM_H
#define CUTEGRAM_H

#include <QObject>
#include <QSize>
#include <QVariantMap>
#include <QSystemTrayIcon>

class CutegramPrivate;
class Cutegram : public QObject
{
    Q_OBJECT
    Q_ENUMS(StartupOptions)

    Q_PROPERTY(QStringList languages READ languages NOTIFY fakeSignal)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY highlightColorChanged)

    Q_PROPERTY(QString language     READ language     WRITE setLanguage     NOTIFY languageChanged    )
    Q_PROPERTY(QString messageAudio READ messageAudio WRITE setMessageAudio NOTIFY messageAudioChanged)
    Q_PROPERTY(QString background   READ background   WRITE setBackground   NOTIFY backgroundChanged  )
    Q_PROPERTY(QString masterColor  READ masterColor  WRITE setMasterColor  NOTIFY masterColorChanged )

    Q_PROPERTY(int  sysTrayCounter  READ sysTrayCounter  WRITE setSysTrayCounter  NOTIFY sysTrayCounterChanged )
    Q_PROPERTY(int  startupOption   READ startupOption   WRITE setStartupOption   NOTIFY startupOptionChanged  )
    Q_PROPERTY(bool notification    READ notification    WRITE setNotification    NOTIFY notificationChanged   )
    Q_PROPERTY(bool minimumDialogs  READ minimumDialogs  WRITE setMinimumDialogs  NOTIFY minimumDialogsChanged )
    Q_PROPERTY(bool showLastMessage READ showLastMessage WRITE setShowLastMessage NOTIFY showLastMessageChanged)
    Q_PROPERTY(bool darkSystemTray  READ darkSystemTray  WRITE setDarkSystemTray  NOTIFY darkSystemTrayChanged )

public:
    enum StartupOptions {
        StartupAutomatic = 0,
        StartupVisible = 1,
        StartupHide = 2
    };

    Cutegram(QObject *parent = 0);
    ~Cutegram();

    Q_INVOKABLE QSize imageSize( const QString & path );
    Q_INVOKABLE qreal htmlWidth( const QString & txt );

    Q_INVOKABLE QString getTimeString( const QDateTime & dt );

    Q_INVOKABLE int showMenu( const QStringList & actions, QPoint point = QPoint() );

    void setSysTrayCounter(int count , bool force = false);
    int sysTrayCounter() const;

    QStringList languages();

    void setLanguage( const QString & lang );
    QString language() const;

    void setStartupOption( int opt );
    int startupOption() const;

    void setNotification(bool stt);
    bool notification() const;

    void setMinimumDialogs(bool stt);
    bool minimumDialogs() const;

    void setShowLastMessage(bool stt);
    bool showLastMessage() const;

    void setDarkSystemTray(bool stt);
    bool darkSystemTray() const;

    void setBackground(const QString &background);
    QString background() const;

    void setMessageAudio(const QString &file);
    QString messageAudio() const;

    void setMasterColor(const QString &color);
    QString masterColor() const;

    QColor highlightColor() const;

public slots:
    void start();
    void restart();
    void logout();
    void close();
    void quit();
    void aboutAseman();
    void about();
    void configure();
    void incomingAppMessage( const QString & msg );
    void active();

signals:
    void backRequest();
    void sysTrayCounterChanged();
    void fakeSignal();
    void languageChanged();
    void languageDirectionChanged();
    void startupOptionChanged();
    void notificationChanged();
    void minimumDialogsChanged();
    void showLastMessageChanged();
    void backgroundChanged();
    void messageAudioChanged();
    void masterColorChanged();
    void highlightColorChanged();
    void darkSystemTrayChanged();

    void configureRequest();
    void aboutAsemanRequest();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void systray_action( QSystemTrayIcon::ActivationReason act );

private:
    void init_systray();
    void showContextMenu();
    QImage generateIcon( const QImage & img, int count );
    void init_languages();

private:
    CutegramPrivate *p;
};

#endif // CUTEGRAM_H
