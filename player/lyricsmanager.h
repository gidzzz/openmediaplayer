#ifndef LYRICSMANAGER_H
#define LYRICSMANAGER_H

#include <QApplication>
#include <QSettings>
#include <QList>
#include <QStringList>
#include <QNetworkConfigurationManager>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

#include "abstractlyricsprovider.h"

class LyricsManager : public QObject
{
    Q_OBJECT

public:
    LyricsManager(QObject *parent = 0);
    ~LyricsManager();

    void fetchLyrics(QString artist, QString title, bool useCache = true);
    void reloadLyrics();
    void storeLyrics(QString artist, QString title, QString lyrics);
    void deleteLyrics(QString artist, QString title);

    static QMap<QString,QString> listProviders();

    static QString cacheFilePath(QString artist, QString title);
    static QString cacheDirPath(QString artist, QString title);
    static QString loadLyrics(QString artist, QString title);
    static bool clearCache();

signals:
    void lyricsFetched(QString lyrics);
    void lyricsInfo(QString message);

public slots:
    void onLyricsFetched(QString lyrics);
    void onLyricsError(QString message);
    void reloadLyricsOverridingCache();

private:
    QString artist;
    QString title;

    QList<QPluginLoader*> loadersList;
    QList<AbstractLyricsProvider*> providersList;

    bool stub;
    bool connectionError;
    int retry;

    void populate();
    void queryNextProvider();
};

#endif // LYRICSMANAGER_H
