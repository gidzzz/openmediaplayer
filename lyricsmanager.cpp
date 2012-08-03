#include "lyricsmanager.h"

#define PLUGINS_DIR "/opt/openmediaplayer/lyrics/"
#define LYRICS_DIR "/home/user/.cache/openmediaplayer/lyrics/"

LyricsManager::LyricsManager(QObject *parent) :
    QObject(parent),
    retry(-1)
{
    qDebug() << "Started loading plugins";

    // A temporary map for all available providers
    QMap<QString,AbstractLyricsProvider*> availableProviders;
    QMap<QString,QPluginLoader*> loadersMap;

    // Load plugins
    QDir pluginsDir = QDir(PLUGINS_DIR);
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader->instance();
        if (plugin) {
            AbstractLyricsProvider *provider = qobject_cast<AbstractLyricsProvider*>(plugin);
            if (provider) {
                qDebug() << "Found plugin" << provider->name();
                availableProviders.insert(provider->name(), provider);
                loadersMap.insert(provider->name(), loader);
            } else {
                qDebug() << "Found unknown plugin";
                loader->unload();
                delete loader;
            }
        } else {
            qDebug() << loader->errorString();
            delete loader;
        }
    }

    // Sort and store the desired providers
    QStringList desiredProviders = QSettings().value("lyrics/providers").toString().split(',', QString::SkipEmptyParts);
    foreach(QString providerName, desiredProviders) {
        if (providerName.startsWith('+')) {
            providerName = providerName.mid(1);
            AbstractLyricsProvider *provider = availableProviders.take(providerName);
            if (provider) {
                qDebug() << "Activating plugin" << provider->name();
                providersList.append(provider);
                loadersList.append(loadersMap.take(providerName));
            }
        }
    }

    // Free unused providers
    foreach(QPluginLoader *loader, loadersMap) {
        loader->unload();
        delete loader;
    }

    // Establish connections with active providers
    foreach(AbstractLyricsProvider *provider, providersList) {
        connect(provider, SIGNAL(fetched(QString)), this, SLOT(onLyricsFetched(QString)));
        connect(provider, SIGNAL(error(QString)), this, SLOT(onLyricsError(QString)));
    }

    qDebug() << "Finished loading plugins";
}

LyricsManager::~LyricsManager()
{
    qDebug() << "Unloading plugins";

    foreach(QPluginLoader* loader, loadersList) {
        loader->unload();
        delete loader;
    }
}

QStringList LyricsManager::listProviders()
{
    QStringList plugins;

    // Load a plugin, read the name, unload, repeat
    QDir pluginsDir = QDir(PLUGINS_DIR);
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            AbstractLyricsProvider *provider = qobject_cast<AbstractLyricsProvider*>(plugin);
            if (provider)
                plugins << provider->name();
            loader.unload();
        }
    }

    return plugins;
}

QString LyricsManager::cacheFilePath(QString artist, QString title)
{
    return cacheDirPath(artist, title) + title.remove('/') + ".txt";
}

QString LyricsManager::cacheDirPath(QString artist, QString title)
{
    Q_UNUSED(title);

    return QString(LYRICS_DIR) + artist.remove('/') + '/';
}

QString LyricsManager::loadLyrics(QString artist, QString title)
{
    QFile file(cacheFilePath(artist, title));

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString lyrics = QTextStream(&file).readAll();
    file.close();

    return lyrics;
}

void LyricsManager::storeLyrics(QString artist, QString title, QString lyrics)
{
    QDir().mkpath(cacheDirPath(artist, title));
    QFile file(cacheFilePath(artist, title));

    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    QTextStream(&file) << lyrics;
    file.close();
}

void LyricsManager::deleteLyrics(QString artist, QString title)
{
    QFile(cacheFilePath(artist, title)).remove();
    QDir().rmdir(cacheDirPath(artist, title));
}

void LyricsManager::fetchLyrics(QString artist, QString title, bool cached)
{
        // Abort a possible pending operation
        if (retry != -1) {
            providersList.at(retry)->abort();
            retry = -1;
        }

        // Store info about the processed song
        this->artist = artist;
        this->title = title;

        // Fetch lyrics from cache or start querying providers
        if (cached && QFile(cacheFilePath(artist, title)).exists()) {
            emit lyricsFetched(loadLyrics(artist, title));
        } else if (QNetworkConfigurationManager().isOnline()) {
            queryNextProvider();
        } else {
            // A little hack waiting for a proper implemention of offline providers.
            foreach(AbstractLyricsProvider *provider, providersList) {
                if (provider->name().startsWith("OMP")) {
                    retry = providersList.size();
                    provider->fetch(artist, title);
                    return;
                }
            }

            emit lyricsError(tr("There is no active Internet connection"));
        }
}

void LyricsManager::queryNextProvider()
{
    // Check whether there are any providers remaining
    if (++retry < providersList.size()) {
        AbstractLyricsProvider *provider = providersList.at(retry);
        qDebug() << "Querying" << provider->name();
        provider->fetch(artist, title);
    } else {
        retry = -1;
        emit lyricsError(tr("Lyrics not found"));
    }
}

void LyricsManager::onLyricsFetched(QString lyrics)
{
    qDebug() << "Lyrics fetched";

    storeLyrics(artist, title, lyrics);

    retry = -1;
    emit lyricsFetched(lyrics);
}

void LyricsManager::onLyricsError(QString message)
{
    qDebug() << "Lyrics error:" << message;

    queryNextProvider();
}
