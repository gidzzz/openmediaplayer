#ifndef ABSTRACTLYRICSPROVIDER_H
#define ABSTRACTLYRICSPROVIDER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class AbstractLyricsProvider : public QObject
{
    Q_OBJECT

public:
    AbstractLyricsProvider() { nam = NULL; }

    virtual QString name() = 0;
    virtual QString description() = 0;

    virtual void fetch(QString artist, QString title) = 0;
    virtual void abort() = 0;

    QNetworkAccessManager *nam;
    QNetworkReply *reply;

signals:
    void fetched(QString lyrics);
    void error(QString message);
};

Q_DECLARE_INTERFACE(AbstractLyricsProvider, "org.openmediaplayer.AbstractLyricsProvider/1.1")

#endif // ABSTRACTLYRICSPROVIDER_H
