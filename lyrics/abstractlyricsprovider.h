#ifndef ABSTRACTLYRICSPROVIDER_H
#define ABSTRACTLYRICSPROVIDER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class AbstractLyricsProvider : public QObject
{
    Q_OBJECT

public:
    virtual QString name() = 0;

    virtual void fetch(QString artistName, QString songName) = 0;
    virtual void abort() = 0;

    QNetworkAccessManager *nam;
    QNetworkReply *reply;

signals:
    void fetched(QString lyrics);
    void error(QString message);
};

Q_DECLARE_INTERFACE(AbstractLyricsProvider, "org.openmediaplayer.AbstractLyricsProvider/1.0")

#endif // ABSTRACTLYRICSPROVIDER_H
