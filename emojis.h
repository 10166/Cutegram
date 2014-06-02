#ifndef EMOJIS_H
#define EMOJIS_H

#include <QObject>

class EmojisPrivate;
class Emojis : public QObject
{
    Q_PROPERTY( QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_OBJECT
public:
    Emojis(QObject *parent = 0);
    ~Emojis();

    void setCurrentTheme( const QString & theme );
    QString currentTheme() const;

    Q_INVOKABLE QString textToEmojiText( const QString & txt );

signals:
    void currentThemeChanged();

private:
    EmojisPrivate *p;
};

#endif // EMOJIS_H
