#ifndef LYRICSMANAGER_H
#define LYRICSMANAGER_H

#include <QApplication>
#include <QSettings>
#include <QList>
#include <QStringList>

#include <QNetworkConfigurationManager>

// TODO: slim this down
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

#include "lyrics/abstractlyricsprovider.h"

class LyricsManager : public QObject
{
    Q_OBJECT

public:
    LyricsManager(QObject *parent = 0);
    ~LyricsManager();

    void fetchLyrics(QString artist, QString title, bool cached = true);

    static QStringList listProviders();

    static QString cachePath(QString artist, QString title);
    static QString loadLyrics(QString artist, QString title);
    static void storeLyrics(QString artist, QString title, QString lyrics);

signals:
    void lyricsFetched(QString lyrics);
    void lyricsError(QString message);

public slots:
    void onLyricsFetched(QString lyrics);
    void onLyricsError(QString message);

private:
    QString artist;
    QString title;

    QList<QPluginLoader*> loadersList;
    QList<AbstractLyricsProvider*> providersList;
    void queryNextProvider();
    int retry;
};

#endif // LYRICSMANAGER_H
