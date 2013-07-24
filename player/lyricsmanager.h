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

    static QMap<QString,QString> listProviders();

    static QString cacheFilePath(QString artist, QString title);
    static QString cacheDirPath(QString artist, QString title);
    static QString loadLyrics(QString artist, QString title);
    static void storeLyrics(QString artist, QString title, QString lyrics);
    static void deleteLyrics(QString artist, QString title);
    static bool clearCache();

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
    bool connectionError;
    int retry;
};

#endif // LYRICSMANAGER_H
